#pragma once

#include <engine/render/render_api/vulkan/VkResourceStructures.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class RenderDevice;
			class CommandPool;
			class CommandBuffer;
			class Queue;
			class Texture;

			class RenderFrame final
			{
			public:
				RenderFrame(RenderDevice& render_device, const Queue& queue);
				~RenderFrame();

				RenderFrame(const RenderFrame&) = delete;
				RenderFrame(RenderFrame&&) = delete;
				RenderFrame& operator=(const RenderFrame&) = delete;
				RenderFrame& operator=(RenderFrame&&) = delete;

				void UpdateBackbuffer(const vk::Extent2D& extent, vk::Format format, vk::Image image);
				Texture& GetBackbuffer();

				CommandBuffer& RequestCommandBuffer(vk::CommandBufferLevel level);

				void ResetFence();
				void WaitFence();

				vk::Semaphore& GetImageAvaliableSemaphore();
				vk::Semaphore& GetRenderFinishedSemaphore();
				vk::Fence& GetFence();

				void Reset();
			private:
				RenderDevice& render_device;

				std::unique_ptr<Texture> backbuffer;
				std::unique_ptr<CommandPool> command_pool;

				vk::Semaphore image_avaliable{};
				vk::Semaphore render_finished{};
				vk::Fence frame_fence{};
			};
		}
	}
}