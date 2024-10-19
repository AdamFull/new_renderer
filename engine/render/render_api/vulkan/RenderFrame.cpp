#include <engine/render/render_api/vulkan/RenderFrame.h>
#include <engine/render/render_api/vulkan/Queue.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>
#include <engine/render/render_api/vulkan/CommandPool.h>
#include <engine/render/render_api/vulkan/ImageView.h>
#include <engine/render/render_api/vulkan/Texture.h>
#include <engine/render/render_api/vulkan/DescriptorPool.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			RenderFrame::RenderFrame(RenderDevice& render_device, const Queue& queue) :
				render_device{ render_device }
			{
				auto command_pool_builder = CommandPoolBuilder();
				command_pool_builder.SetResetMode(CommandBuffer::ResetMode::ResetIndividually);
				command_pool_builder.SetQueueFamily(queue.GetFamilyIndex());
				command_pool = command_pool_builder.BuildUnique(render_device);

				descriptor_allocator = std::make_unique<DescriptorAllocator>(render_device);

				vk::SemaphoreCreateInfo semaphore_ci{};
				image_avaliable = render_device.CreateSemaphore(semaphore_ci);
				render_finished = render_device.CreateSemaphore(semaphore_ci);
				upload_avaliable = render_device.CreateSemaphore(semaphore_ci);

				allocated_sets.reserve(100ull);

				vk::FenceCreateInfo fence_ci{};
				fence_ci.flags = vk::FenceCreateFlagBits::eSignaled;

				frame_fence = render_device.CreateFence(fence_ci);
			}

			RenderFrame::~RenderFrame()
			{
				render_device.Destroy(image_avaliable);
				render_device.Destroy(render_finished);
				render_device.Destroy(upload_avaliable);
				render_device.Destroy(frame_fence);
			}

			void RenderFrame::UpdateBackbuffer(const vk::Extent2D& extent, vk::Format format, vk::Image image)
			{
				auto builder = TextureBuilder();
				builder.SetDebugName("backbuffer");
				builder.SetExternalImage(image);
				builder.SetFlags(ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eRenderTarget);
				builder.SetFormat(format);
				builder.SetExtent({ extent.width, extent.height, 1u });

				backbuffer = builder.BuildUnique(render_device);
			}

			Texture& RenderFrame::GetBackbuffer()
			{
				return *backbuffer;
			}

			CommandBuffer& RenderFrame::RequestCommandBuffer(vk::CommandBufferLevel level)
			{
				return command_pool->RequestCommandBuffer(level);
			}

			DescriptorSet& RenderFrame::RequestDescriptorSet(DescriptorSetLayout& layout)
			{
				auto& new_descriptor_set = allocated_sets.emplace_back(descriptor_allocator->Allocate(layout));
				return *new_descriptor_set;
			}

			void RenderFrame::ResetFence()
			{
				render_device.ResetFences(&frame_fence, 1u);
			}

			void RenderFrame::WaitFence()
			{
				render_device.WaitForFences(&frame_fence, 1u);
			}

			vk::Semaphore& RenderFrame::GetImageAvaliableSemaphore()
			{
				return image_avaliable;
			}

			vk::Semaphore& RenderFrame::GetRenderFinishedSemaphore()
			{
				return render_finished;
			}

			vk::Semaphore& RenderFrame::GetUploadAvaliableSemaphore()
			{
				return upload_avaliable;
			}

			vk::Fence& RenderFrame::GetFence()
			{
				return frame_fence;
			}

			void RenderFrame::Reset()
			{
				//render_device.WaitForFences(&frame_fence, 1u);
				//render_device.ResetFences(&frame_fence, 1u);

				backbuffer->GetImageView().SetImageLayout(vk::ImageLayout::eUndefined);
				command_pool->Reset();

				allocated_sets.clear();
				descriptor_allocator->Reset();
			}
		}
	}
}