#pragma once

#include "gfx_base.h"

namespace edge::gfx {
	class IUniformPool {
	public:
		virtual ~IUniformPool() = default;

		virtual void frame_mark() = 0;
		virtual void reset() = 0;
		virtual auto request_block(uint64_t requested_size) -> edge::gfx::BufferSuballocation = 0;
	};

	[[nodiscard]] auto create_uniform_pool(uint64_t frame_size = 4096ull) -> IUniformPool*;
	[[nodiscard]] auto create_uniform_pool_unique(uint64_t frame_size = 4096ull) -> std::unique_ptr<IUniformPool>;
}