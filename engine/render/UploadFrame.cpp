#include <engine/render/UploadFrame.h>

#include <render_api/GFX.h>

namespace engine
{
	namespace render
	{
		UploadFrame::UploadFrame(gfx::Context& context, const gfx::Queue& queue) :
			context{ context }, queue{ queue }
		{
			command_pool = context.CreateCommandPool(&queue);

			signal_semaphore = context.CreateSemaphore();
			frame_fence = context.CreateFence();
		}

		UploadFrame::~UploadFrame()
		{
			for (auto& buffer : staging_buffers)
			{
				context.DestroyBuffer(buffer);
			}
			staging_buffers.clear();

			if (command_pool)
			{
				context.DestroyCommandPool(command_pool);
			}

			if (signal_semaphore)
			{
				context.DestroySemaphore(signal_semaphore);
			}

			if (frame_fence)
			{
				context.DestroyFence(frame_fence);
			}
		}

		gfx::Buffer& UploadFrame::RequestStagingBuffer(size_t size, size_t* buffer_index)
		{
			if (buffer_index)
			{
				*buffer_index = staging_buffers.size();
			}

			gfx::BufferCreateInfo buffer_create_info{};
			buffer_create_info.usage = gfx::BufferUsageFlag::eStagingBuffer;
			buffer_create_info.size = size;

			auto& new_buffer = staging_buffers.emplace_back(context.CreateBuffer(buffer_create_info));
			return *new_buffer;
		}

		gfx::Buffer& UploadFrame::GetStagingBuffer(size_t buffer_index)
		{
			return *staging_buffers[buffer_index];
		}

		gfx::CommandList& UploadFrame::AcquireOrGetCommandList()
		{
			if (!active_command_list)
			{
				active_command_list = command_pool->Allocate();
			}

			return *active_command_list;
		}

		gfx::SemaphoreSubmitInfo UploadFrame::Submit(gfx::SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count)
		{
			constexpr size_t k_max_wait_semaphores{ 8ull };

			gfx::SubmitInfo submit_info{};

			gfx::SemaphoreSubmitInfo wait_semaphores[k_max_wait_semaphores]{};
			std::copy(wait_infos, wait_infos + wait_semaphore_count, &wait_semaphores[0]);

			submit_info.wait_semaphore_infos = &wait_semaphores[0];
			submit_info.wait_semaphore_count = wait_semaphore_count;

			// Not shure in witch state i have to signal it
			gfx::SemaphoreSubmitInfo signal_semaphore_info{ signal_semaphore, 0ull, gfx::StageFlag::eTransfer };

			submit_info.signal_semaphore_infos = &signal_semaphore_info;
			submit_info.signal_semaphore_count = 1u;

			submit_info.command_lists = &active_command_list;
			submit_info.command_buffer_count = 1u;

			queue.Submit(submit_info, frame_fence);

			gfx::SemaphoreSubmitInfo wait_semaphore_info{ signal_semaphore, 0ull, gfx::StageFlag::eAllTransfer | gfx::StageFlag::eAllGraphics | gfx::StageFlag::eComputeShader | gfx::StageFlag::eDrawIndirect };
			return wait_semaphore_info;
		}

		void UploadFrame::Reset()
		{
			active_command_list = nullptr;

			frame_fence->Wait();
			frame_fence->Reset();

			for (auto& buffer : staging_buffers)
			{
				context.DestroyBuffer(buffer);
			}
			staging_buffers.clear();

			command_pool->Reset();
		}
	}
}