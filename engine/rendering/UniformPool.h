#pragma once

#include "gfx/gfx_base.h"

namespace engine
{
	namespace gfx
	{
		class Context;
	}

	namespace render
	{
		class UniformPoolFrame final
		{
		public:
			UniformPoolFrame(gfx::Context& context, uint64_t whole_size);

			UniformPoolFrame(const UniformPoolFrame&) = delete;
			UniformPoolFrame(UniformPoolFrame&& other);
			~UniformPoolFrame();

			UniformPoolFrame& operator=(const UniformPoolFrame&) = delete;
			UniformPoolFrame& operator=(UniformPoolFrame&&) = delete;

			bool IsExpired() const;
			void Reset() const;
			bool HasEnoughtSpace(uint64_t requested_size) const;
			gfx::BufferSuballocation RequestBlock(uint64_t requested_size);
		private:
			gfx::Context& context;

			gfx::Buffer* buffer{ nullptr };
			mutable uint64_t size_remain{ 0ull };

			mutable int32_t expired_countdown{ 0 };
		};

		class UniformPool final
		{
		public:
			UniformPool(gfx::Context& context, uint64_t frame_size = 4096ull);

			UniformPool(const UniformPool&) = delete;
			UniformPool(UniformPool&& other);
			~UniformPool() = default;

			UniformPool& operator=(const UniformPool&) = delete;
			UniformPool& operator=(UniformPool&&) = delete;

			void Reset();
			gfx::BufferSuballocation RequestBlock(uint64_t requested_size);
		private:
			gfx::Context& context;

			uint64_t minimal_alignment{ 1ull };
			uint64_t frame_size{ 4096ull };
			std::vector<std::unique_ptr<UniformPoolFrame>> pool_frames;
		};
	}
}