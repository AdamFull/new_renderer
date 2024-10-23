#pragma once

#include <array>
#include <memory>

namespace engine
{
	namespace gfx
	{
		class Context;
		class Queue;
		class Fence;
		class Semaphore;
		class CommandList;
		class CommandPool;
		class Texture;
	}

	namespace render
	{
		class RenderFrame final
		{
		public:
			RenderFrame(gfx::Context& gfx_context, const gfx::Queue& queue);
			~RenderFrame();

			RenderFrame(const RenderFrame&) = delete;
			RenderFrame(RenderFrame&&) = delete;
			RenderFrame& operator=(const RenderFrame&) = delete;
			RenderFrame& operator=(RenderFrame&&) = delete;

			void UpdateBackbuffer(gfx::Texture* new_backbuffer);
		private:
			gfx::Context& context;

			gfx::Texture* backbuffer{ nullptr };
			gfx::CommandPool* command_pool{ nullptr };

			//std::unique_ptr<DescriptorAllocator> descriptor_allocator;

			//std::unique_ptr<UniformPool> uniform_pool{};

			//DescriptorManagementStrategy descriptor_management_strategy{ DescriptorManagementStrategy::StoreInCache };

			gfx::Semaphore* image_avaliable{ nullptr };
			gfx::Semaphore* render_finished{ nullptr };
			gfx::Semaphore* upload_avaliable{ nullptr };
			gfx::Fence* frame_fence{ nullptr };
		};
	}
}