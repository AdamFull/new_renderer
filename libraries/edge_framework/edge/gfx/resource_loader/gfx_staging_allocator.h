#pragma once

#include <vector>
#include <string_view>

namespace edge
{
	namespace gfx
	{
		class IContext;
		class IBuffer;

		using Suballocation = std::tuple<IBuffer*, uint64_t>;

		class StagingBufferAllocator
		{
		public:
			StagingBufferAllocator(IContext* gfx_context);
			virtual ~StagingBufferAllocator() = default;

			StagingBufferAllocator(StagingBufferAllocator&& other) noexcept;
			auto operator=(StagingBufferAllocator&& other) noexcept -> StagingBufferAllocator&;

			StagingBufferAllocator(const StagingBufferAllocator&) = delete;
			StagingBufferAllocator& operator=(const StagingBufferAllocator&) = delete;

			virtual auto Allocate(uint64_t requested_size, uint64_t alignment = 1ull) -> Suballocation = 0;
			virtual auto Reset() -> void = 0;
		protected:
			auto AllocateBuffer(uint64_t size, std::string_view name = {}) -> IBuffer*;

			IContext* gfx_context;
		};

		class DummyStagingAllocator final : public StagingBufferAllocator
		{
		public:
			DummyStagingAllocator(IContext* gfx_context, uint64_t size);
			~DummyStagingAllocator() override;

			DummyStagingAllocator(DummyStagingAllocator&& other) noexcept;
			auto operator=(DummyStagingAllocator&& other) noexcept -> DummyStagingAllocator&;

			auto Allocate(uint64_t requested_size, uint64_t alignment = 1ull) -> Suballocation override;
			auto Reset() -> void override;
		private:
			std::vector<IBuffer*> buffers{};
		};

		class ArenaStagingAllocator final : public StagingBufferAllocator
		{
		public:
			struct Block
			{
				IBuffer* buffer;
				uint64_t used;
			};

			ArenaStagingAllocator(IContext* gfx_context, uint64_t size);
			~ArenaStagingAllocator() override;

			ArenaStagingAllocator(ArenaStagingAllocator&& other) noexcept;
			auto operator=(ArenaStagingAllocator&& other) noexcept -> ArenaStagingAllocator&;

			auto Allocate(uint64_t requested_size, uint64_t alignment = 1ull) -> Suballocation override;
			auto Reset() -> void override;

		private:
			auto GetBlock(uint64_t requested_size) -> Block&;
			auto AddBlock(uint64_t size) -> Block&;

			uint64_t block_size{ 0ull };
			std::vector<Block> blocks{};
		};
	}
}