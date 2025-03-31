#pragma once

#include <edge/gfx/gfx_base.h>

namespace engine
{
	namespace render
	{
		class UniformPoolFrame final
		{
		public:
			UniformPoolFrame(uint64_t whole_size);

			UniformPoolFrame(const UniformPoolFrame&) = delete;
			UniformPoolFrame(UniformPoolFrame&& other);
			~UniformPoolFrame();

			UniformPoolFrame& operator=(const UniformPoolFrame&) = delete;
			UniformPoolFrame& operator=(UniformPoolFrame&&) = delete;

			bool IsExpired() const;
			void Reset() const;
			bool HasEnoughtSpace(uint64_t requested_size) const;
			edge::gfx::BufferSuballocation RequestBlock(uint64_t requested_size);
		private:
			edge::gfx::IBuffer* buffer{ nullptr };
			mutable uint64_t size_remain{ 0ull };

			mutable int32_t expired_countdown{ 0 };
		};

		class UniformPool final
		{
		public:
			UniformPool(uint64_t frame_size = 4096ull);

			UniformPool(const UniformPool&) = delete;
			UniformPool(UniformPool&& other);
			~UniformPool() = default;

			UniformPool& operator=(const UniformPool&) = delete;
			UniformPool& operator=(UniformPool&&) = delete;

			void Reset();
			edge::gfx::BufferSuballocation RequestBlock(uint64_t requested_size);
		private:
			uint64_t minimal_alignment{ 1ull };
			uint64_t frame_size{ 4096ull };
			std::vector<std::unique_ptr<UniformPoolFrame>> pool_frames;
		};
	}
}