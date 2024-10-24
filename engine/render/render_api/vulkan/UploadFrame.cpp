#include <engine/render/render_api/vulkan/UploadFrame.h>
#include <base/Queue.h>
#include <base/CommandPool.h>
#include <RenderDevice.h>
#include <base/Buffer.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			UploadFrame::UploadFrame(RenderDevice& render_device, const Queue& queue) :
				render_device{ render_device },
				queue{ queue }
			{
				auto command_pool_builder = CommandPoolBuilder();
				command_pool_builder.SetResetMode(CommandBuffer::ResetMode::ResetIndividually);
				command_pool_builder.SetQueueFamily(queue.GetFamilyIndex());
				command_pool = command_pool_builder.BuildUnique(render_device);

				vk::SemaphoreCreateInfo semaphore_ci{};
				signal_semaphore = render_device.CreateSemaphore(semaphore_ci);

				vk::FenceCreateInfo fence_ci{};
				fence_ci.flags = vk::FenceCreateFlagBits::eSignaled;

				frame_fence = render_device.CreateFence(fence_ci);
			}

			UploadFrame::~UploadFrame()
			{
				render_device.Destroy(signal_semaphore);
				render_device.Destroy(frame_fence);
			}

			Buffer& UploadFrame::RequestStagingBuffer(vk::DeviceSize size, size_t* buffer_index)
			{
				auto builder = BufferBuilder(size, 1ull, 1ull);
				builder.SetVmaUsage(vma::MemoryUsage::eCpuOnly);
				builder.SetUsage(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
				builder.SetSharingMode(vk::SharingMode::eConcurrent);

				if (buffer_index)
				{
					*buffer_index = staging_buffers.size();
				}

				auto& buffer = staging_buffers.emplace_back(builder.BuildUnique(render_device));
				return *buffer;
			}

			Buffer& UploadFrame::GetStagingBuffer(size_t buffer_index)
			{
				return *staging_buffers[buffer_index];
			}

			CommandBuffer& UploadFrame::AcquireOrGetCommandBuffer(vk::CommandBufferLevel level)
			{
				if (!active_command_buffer)
				{
					auto& command_buffer = command_pool->RequestCommandBuffer(level);
					active_command_buffer = &command_buffer;

					command_buffer.Begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
				}

				return *active_command_buffer;
			}

			vk::SemaphoreSubmitInfo UploadFrame::Submit(vk::SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count)
			{
				constexpr size_t k_max_wait_semaphores{ 8ull };

				vk::SubmitInfo2 submit_info{};

				vk::SemaphoreSubmitInfo wait_semaphores[k_max_wait_semaphores]{};
				std::copy(wait_infos, wait_infos + wait_semaphore_count, &wait_semaphores[0]);

				submit_info.pWaitSemaphoreInfos = &wait_semaphores[0];
				submit_info.waitSemaphoreInfoCount = wait_semaphore_count;

				// Not shure in witch state i have to signal it
				vk::SemaphoreSubmitInfo signal_semaphore_info{ signal_semaphore, 0ull, vk::PipelineStageFlagBits2::eTransfer };

				submit_info.pSignalSemaphoreInfos = &signal_semaphore_info;
				submit_info.signalSemaphoreInfoCount = 1;

				vk::CommandBufferSubmitInfo command_buffer_si{};
				command_buffer_si.commandBuffer = Handle(*active_command_buffer);

				submit_info.pCommandBufferInfos = &command_buffer_si;
				submit_info.commandBufferInfoCount = 1;

				queue.Submit(submit_info, frame_fence);

				vk::SemaphoreSubmitInfo wait_semaphore_info{ signal_semaphore, 0ull, vk::PipelineStageFlagBits2::eAllTransfer | vk::PipelineStageFlagBits2::eAllGraphics | vk::PipelineStageFlagBits2::eComputeShader | vk::PipelineStageFlagBits2::eDrawIndirect };
				return wait_semaphore_info;
			}

			void UploadFrame::Reset()
			{
				active_command_buffer = nullptr;

				render_device.WaitForFences(&frame_fence, 1u);
				render_device.ResetFences(&frame_fence, 1u);

				staging_buffers.clear();

				command_pool->Reset();
			}
		}
	}
}