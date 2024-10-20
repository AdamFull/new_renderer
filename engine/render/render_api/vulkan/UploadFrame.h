#pragma once

#include <ResourceStructures.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Queue;
			class RenderDevice;
			class CommandPool;
			class CommandBuffer;
			class Buffer;

			class UploadFrame final
			{
			public:
				UploadFrame(RenderDevice& render_device, const Queue& queue);
				~UploadFrame();

				UploadFrame(const UploadFrame&) = delete;
				UploadFrame(UploadFrame&&) = delete;
				UploadFrame& operator=(const UploadFrame&) = delete;
				UploadFrame& operator=(UploadFrame&&) = delete;

				Buffer& RequestStagingBuffer(vk::DeviceSize size, size_t* buffer_index = nullptr);
				Buffer& GetStagingBuffer(size_t buffer_index);

				CommandBuffer& AcquireOrGetCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

				vk::SemaphoreSubmitInfo Submit(vk::SemaphoreSubmitInfo* wait_infos, uint32_t semaphore_count);

				void Reset();
			private:
				RenderDevice& render_device;

				const Queue& queue;

				std::unique_ptr<CommandPool> command_pool;
				CommandBuffer* active_command_buffer{ nullptr };

				std::vector<std::unique_ptr<Buffer>> staging_buffers;

				vk::Semaphore signal_semaphore{};
				vk::Fence frame_fence{};
			};
		}
	}
}