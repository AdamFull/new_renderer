#pragma once

#include <engine/rendering/gfx/Context.h>
#include <engine/rendering/gfx/ResourceLoader.h>

#include <util/Util.h>

namespace engine
{
	namespace render
	{
		template<typename _Ty>
		class DynamicBuffer final
		{
		public:
			struct DirtyRange
			{
				uint32_t first;
				uint32_t count;
			};

			DynamicBuffer(gfx::ResourceContext& resource_context, uint64_t capacity = 64u, gfx::BufferUsageFlags flags = {}) :
				resource_context{ resource_context },
				context{ resource_context.GetContext() },
				usage_flags{ flags }
			{
				gfx::BufferCreateInfo create_info{};
				create_info.size = sizeof(_Ty) * capacity;
				create_info.usage = flags;

				buffer = context.CreateBuffer(create_info);
				memory.resize(create_info.size);
			}

			~DynamicBuffer()
			{
				context.Destroy(buffer);
			}

			// Mark range as dirty
			void MarkDirty(uint32_t index, uint32_t count = 1u)
			{
				dirty_ranges.emplace_back(index, count);
			}

			_Ty& Get(uint32_t index)
			{
				if (index > memory.size())
				{
					memory.resize(memory.size() + index);
				}

				return memory[index];
			}

			_Ty* Data(uint32_t offset = 0u)
			{
				if (offset > memory.size())
				{
					memory.resize(memory.size() + offset);
				}

				return memory.data() + offset;
			}

			void Flush()
			{
				if (!dirty_ranges.empty())
				{
					auto whole_size = memory.size() * sizeof(_Ty);
					if (buffer->GetSize() < whole_size)
					{
						// Update buffer size
						gfx::BufferCreateInfo create_info{};
						create_info.size = whole_size;
						create_info.usage = usage_flags;

						context.Destroy(buffer);
						buffer = context.CreateBuffer(create_info);

						dirty_ranges.clear();
						dirty_ranges.emplace_back(0u, static_cast<uint32_t>(memory.size()));
					}

					std::sort(dirty_ranges.begin(), dirty_ranges.end(), [](const DirtyRange& a, const DirtyRange& b) { return a.first < b.first; });

					// Merge dirty ranges
					std::vector<DirtyRange> merged_ranges{};
					merged_ranges.emplace_back(dirty_ranges.front());

					for (int32_t i = 1; i < dirty_ranges.size(); ++i)
					{
						auto& last_range = merged_ranges.back();
						if (dirty_ranges[i].first == last_range.first + last_range.count)
							last_range.count = std::max(last_range.count, dirty_ranges[i].first + dirty_ranges[i].count - last_range.first);
						else
							merged_ranges.emplace_back(dirty_ranges[i]);
					}

					gfx::BufferUpdateInfo update_info{};
					update_info.dst_buffer = buffer;
					update_info.update_size = std::accumulate(merged_ranges.begin(), merged_ranges.end(), 0, [](uint64_t acc, const DirtyRange& item) { return acc + item.count * sizeof(_Ty); });
					resource_context.BeginBufferUpdate(update_info);

					uint64_t src_offset{ 0ull };
					for (const auto& range : merged_ranges)
					{
						auto range_offset = range.first * sizeof(_Ty);
						auto range_size = range.count * sizeof(_Ty);;

						auto& region = update_info.regions[update_info.region_count++];
						region.src_offset = src_offset;
						region.dst_offset = range_offset;
						region.size = range_size;
					
						uint8_t* range_start = reinterpret_cast<uint8_t*>(&memory[range.first]);
						util::Copy(update_info.map_info.data, src_offset, memory.data(), range_size, range_offset);
					
						src_offset += range_size;
					}
					
					resource_context.EndBufferUpdate(update_info);
					dirty_ranges.clear();
				}
			}
		private:
			gfx::Context& context;
			gfx::ResourceContext& resource_context;

			gfx::BufferUsageFlags usage_flags{};

			gfx::Buffer* buffer{ nullptr };
			std::vector<_Ty> memory{};

			std::vector<DirtyRange> dirty_ranges{};
		};
	}
}