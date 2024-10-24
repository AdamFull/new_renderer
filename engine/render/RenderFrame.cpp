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

		gfx::Texture& RenderFrame::GetBackbuffer()
		{
			return *backbuffer;
		}

		gfx::CommandList& RenderFrame::RequestCommandList(bool secondary)
		{
			return *command_pool->Allocate(secondary);
		}

		//DescriptorSet& RequestDescriptorSet(DescriptorSetLayout& layout);

		//BufferSuballocation RequestUniformBlock(vk::DeviceSize requested_size);

		void RenderFrame::ResetFence()
		{
			frame_fence->Reset();
		}

		void RenderFrame::WaitFence()
		{
			frame_fence->Wait();
		}

		gfx::Semaphore& RenderFrame::GetImageAvaliableSemaphore()
		{
			return *image_avaliable;
		}

		gfx::Semaphore& RenderFrame::GetRenderFinishedSemaphore()
		{
			return *render_finished;
		}

		gfx::Semaphore& RenderFrame::GetUploadAvaliableSemaphore()
		{
			return *upload_avaliable;
		}

		gfx::Fence& RenderFrame::GetFence()
		{
			return *frame_fence;
		}

		void RenderFrame::Reset()
		{
			backbuffer->Reset();
			command_pool->Reset();

			//uniform_pool->Reset();

			//descriptor_allocator->Reset();
		}
	}
}