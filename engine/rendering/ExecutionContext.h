#pragma once

#include <engine/rendering/gfx/gfx_base.h>

namespace engine
{
	namespace render
	{
		class ExecutionContext
		{
		public:
			virtual ~ExecutionContext() = default;

			virtual gfx::BufferSuballocation RequestUniformBlock(uint64_t block_size) = 0;
			virtual gfx::CommandList& GetCurrentCommandList() = 0;
			virtual gfx::DescriptorAllocator& GetCurrentDescriptorAllocator() = 0;
		};
	}
}