#include "gfx_staging_allocator.h"

#include "../gfx_context.h"

#include "../../util/Memory.hpp"

namespace edge
{
	namespace gfx
	{
		StagingBufferAllocator::StagingBufferAllocator(IContext* gfx_context) :
			gfx_context{ gfx_context }
		{
		}

		StagingBufferAllocator::StagingBufferAllocator(StagingBufferAllocator&& other) noexcept :
			gfx_context{ std::exchange(other.gfx_context, nullptr) }
		{
		}

		auto StagingBufferAllocator::operator=(StagingBufferAllocator&& other) noexcept -> StagingBufferAllocator&
		{
			gfx_context = std::exchange(other.gfx_context, nullptr);
			return *this;
		}

		auto StagingBufferAllocator::AllocateBuffer(uint64_t size, std::string_view name) -> IBuffer*
		{
			BufferCreateInfo create_info{};
			create_info.size = size;
			create_info.usage = BufferUsageFlag::eStagingBuffer;

			auto* new_buffer = gfx_context->CreateBuffer(create_info);

			if (!name.empty())
			{
				new_buffer->SetResourceName(name);
			}

			return new_buffer;
		}

		// Dummy allocator
		DummyStagingAllocator::DummyStagingAllocator(IContext* gfx_context, uint64_t size) :
			StagingBufferAllocator{ gfx_context }
		{
		}

		DummyStagingAllocator::~DummyStagingAllocator()
		{
			Reset();
		}

		DummyStagingAllocator::DummyStagingAllocator(DummyStagingAllocator&& other) noexcept :
			StagingBufferAllocator{ std::move(other) },
			buffers{ std::exchange(other.buffers, {}) }
		{
		}

		auto DummyStagingAllocator::operator=(DummyStagingAllocator&& other) noexcept -> DummyStagingAllocator&
		{
			StagingBufferAllocator::operator=(std::move(other));
			buffers = std::exchange(other.buffers, {});

			return *this;
		}

		auto DummyStagingAllocator::Allocate(uint64_t requested_size, uint64_t alignment) -> Suballocation
		{
			uint64_t size = aligned_size(requested_size, alignment);
			return { buffers.emplace_back(AllocateBuffer(size)), 0ull };
		}

		auto DummyStagingAllocator::Reset() -> void
		{
			for (auto& buffer : buffers)
			{
				gfx_context->Destroy(buffer);
			}
		}

		// Arena allocator
		ArenaStagingAllocator::ArenaStagingAllocator(IContext* gfx_context, uint64_t size) :
			StagingBufferAllocator{ gfx_context }
		{
			AddBlock(size);
		}

		ArenaStagingAllocator::~ArenaStagingAllocator()
		{
			for (auto& block : blocks)
			{
				gfx_context->Destroy(block.buffer);
			}
		}

		ArenaStagingAllocator::ArenaStagingAllocator(ArenaStagingAllocator&& other) noexcept :
			StagingBufferAllocator{ std::move(other) },
			block_size{ std::exchange(other.block_size, 0ull) },
			blocks{ std::exchange(other.blocks, {}) }
		{
		}

		auto ArenaStagingAllocator::operator=(ArenaStagingAllocator&& other) noexcept -> ArenaStagingAllocator&
		{
			StagingBufferAllocator::operator=(std::move(other));
			block_size = std::exchange(other.block_size, 0ull);
			blocks = std::exchange(other.blocks, {});

			return *this;
		}

		auto ArenaStagingAllocator::Allocate(uint64_t requested_size, uint64_t alignment) -> Suballocation
		{
			uint64_t size = aligned_size(requested_size, alignment);

			Block& block = GetBlock(size);
			block.used += size;
			return { block.buffer, aligned_size(block.used - size, alignment) };
		}

		auto ArenaStagingAllocator::Reset() -> void
		{
			for (auto& block : blocks)
				block.used = 0;
		}

		auto ArenaStagingAllocator::GetBlock(uint64_t requested_size) -> Block&
		{
			auto found_block = std::find_if(blocks.begin(), blocks.end(), [requested_size](const Block& block) {
				return  block.buffer->GetSize() - block.used > requested_size;
				});

			if (found_block != blocks.end())
				return *found_block;

			return AddBlock(std::max(block_size, requested_size));
		}

		auto ArenaStagingAllocator::AddBlock(uint64_t size) -> Block&
		{
			auto& block = blocks.emplace_back(Block{
				.buffer = AllocateBuffer(size, "arena_staging_buffer"),
				.used = 0ull
				});

			return block;
		}
	}
}