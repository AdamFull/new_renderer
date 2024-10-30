#include "DynamicBuffer.h"

#include <engine/rendering/gfx/ResourceLoader.h>
#include <engine/rendering/gfx/Context.h>

#include <util/Util.h>
#include <hash.hpp>

namespace engine
{
	namespace render
	{
		DynamicBuffer::DynamicBuffer(gfx::ResourceContext& resource_context, uint64_t element_size, uint64_t capacity, gfx::BufferUsageFlags flags) :
			resource_context{ resource_context },
			element_size{ element_size },
			usage_flags{ flags }
		{
			memory.resize(element_size * capacity);
		}

		DynamicBuffer::~DynamicBuffer()
		{
			auto& context = resource_context.GetContext();
			context.Destroy(buffer);
		}

		void DynamicBuffer::Flush()
		{
			if (!buffer)
			{
				gfx::BufferCreateInfo create_info{};
				create_info.size = memory.size();
				create_info.usage = usage_flags;

				auto& context = resource_context.GetContext();
				buffer = context.CreateBuffer(create_info);
			}

			if (!memory.empty())
			{
				// Buffer size is too small, need to resize it
				if (buffer->GetSize() < memory.size())
				{
					gfx::BufferCreateInfo create_info{};
					create_info.size = memory.size();
					create_info.usage = usage_flags;

					auto& context = resource_context.GetContext();
					context.Destroy(buffer);
					buffer = context.CreateBuffer(create_info);

					// Buffer was resized. We should upload whole data
					insertion_ranges.clear();
					insertion_ranges.emplace_back( 0ull, memory.size());
				}

				// Sort upload ranges
				std::sort(insertion_ranges.begin(), insertion_ranges.end(), [](const InsertionRange& a, const InsertionRange& b) { return a.offset < b.offset; });

				// Merge upload ranges
				std::vector<InsertionRange> merged_ranges{};
				merged_ranges.emplace_back(insertion_ranges.front());

				for (int32_t i = 1; i < insertion_ranges.size(); ++i)
				{
					auto& last_range = merged_ranges.back();
					if (insertion_ranges[i].offset == last_range.offset + last_range.size)
						last_range.size = std::max(last_range.size, insertion_ranges[i].offset + insertion_ranges[i].size - last_range.offset);
					else
						merged_ranges.emplace_back(insertion_ranges[i]);
				}

				gfx::BufferUpdateInfo update_info{};
				update_info.dst_buffer = buffer;
				update_info.update_size = std::accumulate(merged_ranges.begin(), merged_ranges.end(), 0, [](uint64_t acc, const InsertionRange& item) { return acc + item.size; });
				resource_context.BeginBufferUpdate(update_info);

				uint64_t src_offset{ 0ull };
				for (const auto& range : merged_ranges)
				{
					auto& region = update_info.regions[update_info.region_count++];
					region.src_offset = src_offset;
					region.dst_offset = range.offset;
					region.size = range.size;

					uint8_t* range_start = &memory[range.offset];
					util::Copy(update_info.map_info.data, src_offset, memory.data(), range.size, range.offset);

					src_offset += range.size;
				}

				resource_context.EndBufferUpdate(update_info);
				insertion_ranges.clear();
			}
		}

		void DynamicBuffer::TryGrow(uint64_t offset, uint64_t size)
		{
			if (offset + size > memory.size())
			{
				uint64_t new_size = (offset + size) * 2.f;
				memory.resize(new_size);
			}
		}

		uint64_t DynamicBuffer::CalculateOffset(uint64_t index)
		{
			return element_size * index;
		}

		void DynamicBuffer::InsertBytes(const void* data, uint64_t size, uint64_t offset)
		{
			auto new_hash = utl::murmur_hash(reinterpret_cast<const char*>(data), size, 0);
			auto old_hash = utl::murmur_hash(reinterpret_cast<const char*>(&memory[offset]), size, 0);

			if (new_hash != old_hash)
			{
				util::Copy(&memory[0], offset, data, size, 0ull, std::execution::par);
				insertion_ranges.emplace_back( offset, size );
			}
		}

		void* DynamicBuffer::GetBytes(uint64_t offset)
		{
			return &memory[offset];
		}
	}
}