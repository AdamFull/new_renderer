#include "gfx_staging_allocator.h"

#include "../gfx_context.h"

#include "../../util/utl_memory.hpp"

namespace edge::gfx
{
	auto StagingBufferAllocator::allocate_buffer(uint64_t size, std::string_view name) -> IBuffer* {
		BufferCreateInfo create_info{};
		create_info.size = size;
		create_info.usage = BufferUsageFlag::eStagingBuffer;

		auto* new_buffer = create_buffer(create_info);

		if (!name.empty()) {
			new_buffer->set_resource_name(name);
		}

		return new_buffer;
	}

	// Dummy allocator
	DummyStagingAllocator::DummyStagingAllocator(uint64_t size) :
		StagingBufferAllocator{} {
	}

	DummyStagingAllocator::~DummyStagingAllocator() {
		reset();
	}

	DummyStagingAllocator::DummyStagingAllocator(DummyStagingAllocator&& other) noexcept :
		buffers{ std::exchange(other.buffers, {}) } {
	}

	auto DummyStagingAllocator::operator=(DummyStagingAllocator&& other) noexcept -> DummyStagingAllocator& {
		buffers = std::exchange(other.buffers, {});

		return *this;
	}

	auto DummyStagingAllocator::allocate(uint64_t requested_size, uint64_t alignment) -> Suballocation {
		uint64_t size = aligned_size(requested_size, alignment);
		return { buffers.emplace_back(allocate_buffer(size)), 0ull };
	}

	auto DummyStagingAllocator::reset() -> void {
		for (auto& buffer : buffers) {
			destroy(buffer);
		}
	}

	// Arena allocator
	ArenaStagingAllocator::ArenaStagingAllocator(uint64_t size) :
		StagingBufferAllocator{} {
		add_block(size);
	}

	ArenaStagingAllocator::~ArenaStagingAllocator() {
		for (auto& block : blocks) {
			destroy(block.buffer);
		}
	}

	ArenaStagingAllocator::ArenaStagingAllocator(ArenaStagingAllocator&& other) noexcept :
		block_size{ std::exchange(other.block_size, 0ull) },
		blocks{ std::exchange(other.blocks, {}) } {
	}

	auto ArenaStagingAllocator::operator=(ArenaStagingAllocator&& other) noexcept -> ArenaStagingAllocator& {
		block_size = std::exchange(other.block_size, 0ull);
		blocks = std::exchange(other.blocks, {});

		return *this;
	}

	auto ArenaStagingAllocator::allocate(uint64_t requested_size, uint64_t alignment) -> Suballocation {

		uint64_t size = aligned_size(requested_size, alignment);
		Block& block = get_block(size);
		block.used += size;
		return { block.buffer, aligned_size(block.used - size, alignment) };
	}

	auto ArenaStagingAllocator::reset() -> void {
		for (auto& block : blocks)
			block.used = 0;
	}

	auto ArenaStagingAllocator::get_block(uint64_t requested_size) -> Block& {
		auto found_block = std::find_if(blocks.begin(), blocks.end(), [requested_size](const Block& block) {
			return  block.buffer->get_size() - block.used > requested_size;
			});

		if (found_block != blocks.end())
			return *found_block;

		return add_block(std::max(block_size, requested_size));
	}

	auto ArenaStagingAllocator::add_block(uint64_t size) -> Block&
	{
		auto& block = blocks.emplace_back(Block{
			.buffer = allocate_buffer(size, "arena_staging_buffer"),
			.used = 0ull
			});

		return block;
	}
}