#pragma once

#include <ResourceStructures.h>
#include <UniformPool.h>

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
			class DescriptorAllocator;
			class DescriptorSet;
			class DescriptorSetLayout;
			class UniformPool;

			enum class DescriptorManagementStrategy
			{
				StoreInCache,
				CreateDirectly
			};

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

				DescriptorSet& RequestDescriptorSet(DescriptorSetLayout& layout);

				BufferSuballocation RequestUniformBlock(vk::DeviceSize requested_size);

				void ResetFence();
				void WaitFence();

				vk::Semaphore& GetImageAvaliableSemaphore();
				vk::Semaphore& GetRenderFinishedSemaphore();
				vk::Semaphore& GetUploadAvaliableSemaphore();
				vk::Fence& GetFence();

				void Reset();
			private:
				RenderDevice& render_device;

				std::unique_ptr<Texture> backbuffer;
				std::unique_ptr<CommandPool> command_pool;

				std::unique_ptr<DescriptorAllocator> descriptor_allocator;

				std::unique_ptr<UniformPool> uniform_pool{};

				DescriptorManagementStrategy descriptor_management_strategy{ DescriptorManagementStrategy::StoreInCache };

				vk::Semaphore image_avaliable{};
				vk::Semaphore render_finished{};
				vk::Semaphore upload_avaliable{};
				vk::Fence frame_fence{};
			};
		}
	}
}