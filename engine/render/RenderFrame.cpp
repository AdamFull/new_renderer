#include <engine/render/RenderFrame.h>

#include <render_api/GFX.h>

namespace engine
{
	namespace render
	{
		RenderFrame::RenderFrame(gfx::Context& context, const gfx::Queue& queue) :
			context{ context }
		{
			command_pool = context.CreateCommandPool(&queue);

			//descriptor_allocator = std::make_unique<DescriptorAllocator>(render_device);

			//uniform_pool = std::make_unique<UniformPool>(render_device, 16384ull);

			image_avaliable = context.CreateSemaphore();
			render_finished = context.CreateSemaphore();
			upload_avaliable = context.CreateSemaphore();

			frame_fence = context.CreateFence();
		}

		RenderFrame::~RenderFrame()
		{
			if (backbuffer)
			{
				context.DestroyTexture(backbuffer);
			}

			if (image_avaliable)
			{
				context.DestroySemaphore(image_avaliable);
			}

			if (render_finished)
			{
				context.DestroySemaphore(render_finished);
			}

			if (upload_avaliable)
			{
				context.DestroySemaphore(upload_avaliable);
			}

			if (frame_fence)
			{
				context.DestroyFence(frame_fence);
			}
		}

		void RenderFrame::UpdateBackbuffer(gfx::Texture* new_backbuffer)
		{
			context.DestroyTexture(backbuffer);
			backbuffer = new_backbuffer;
		}
	}
}