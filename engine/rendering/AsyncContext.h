#pragma once

#include "ExecutionContext.h"
#include <span>

#include <edge/gfx/gfx_base.h>

namespace engine
{
	namespace render
	{
		class IRenderer;

		class AsyncContext final : public ExecutionContext
		{
		public:
			AsyncContext(IRenderer& renderer, edge::Window* window);
			~AsyncContext();

			AsyncContext(const AsyncContext&) = delete;
			AsyncContext(AsyncContext&&) = delete;
			AsyncContext& operator=(const AsyncContext&) = delete;
			AsyncContext& operator=(AsyncContext&&) = delete;
		private:
			edge::gfx::Context& context;
			//edge::gfx::ext::ResourceLoader& resource_loader;
		};
	}
}