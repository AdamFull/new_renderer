#pragma once

#include <edge/gfx/gfx_context.h>
#include <edge/gfx/ext/resource_loader/gfx_resource_loader.h>

#include <edge/util/util.h>

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

			DynamicBuffer(edge::gfx::ext::ResourceLoader& resource_loader, uint64_t capacity = 64u, edge::gfx::BufferUsageFlags flags = {}) :
				resource_loader{ resource_loader },
				context{ resource_loader.GetContext() },
				usage_flags{ flags }
			{
				edge::gfx::BufferCreateInfo create_info{};
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
				if (index >= memory.size())
				{
					memory.resize(memory.size() + index);
				}

				return memory[index];
			}

			_Ty* Data(uint32_t offset = 0u)
			{
				if (offset >= memory.size())
				{
					memory.resize(memory.size() + offset);
				}

				return memory.data() + offset;
			}

			edge::gfx::Buffer* GetBuffer()
			{
				return buffer;
			}

			void Flush()
			{
				if (!dirty_ranges.empty())
				{
					auto whole_size = memory.size() * sizeof(_Ty);
					if (buffer->GetSize() < whole_size)
					{
						// Update buffer size
						edge::gfx::BufferCreateInfo create_info{};
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

					edge::gfx::BufferUpdateInfo update_info{};
					update_info.dst_buffer = buffer;
					update_info.update_size = std::accumulate(merged_ranges.begin(), merged_ranges.end(), 0, [](uint64_t acc, const DirtyRange& item) { return acc + item.count * sizeof(_Ty); });
					auto* updater = resource_loader.BeginBufferUpdate(update_info);

					uint64_t src_offset{ 0ull };
					for (const auto& range : merged_ranges)
					{
						auto range_offset = range.first * sizeof(_Ty);
						auto range_size = range.count * sizeof(_Ty);

						updater->Update(memory.data(), range_size, range_offset);
					}
					
					resource_loader.EndBufferUpdate(&updater);
					dirty_ranges.clear();
				}
			}
		private:
			edge::gfx::Context& context;
			edge::gfx::ext::ResourceLoader& resource_loader;

			edge::gfx::BufferUsageFlags usage_flags{};

			edge::gfx::Buffer* buffer{ nullptr };
			std::vector<_Ty> memory{};

			std::vector<DirtyRange> dirty_ranges{};
		};
	}
}