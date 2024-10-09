#include <engine/render/render_api/vulkan/RenderContext.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/Swapchain.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>

#include <engine/runtime/Window.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			RenderContext::RenderContext(Window* window, RenderDevice& render_device) :
				render_device{ render_device }
			{
#ifdef VK_USE_PLATFORM_ANDROID_KHR
				vk::PresentModeKHR present_mode = (window->GetProperties().vsync == Window::Vsync::OFF) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
				std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate };
#else
				vk::PresentModeKHR present_mode = (window->GetProperties().vsync == Window::Vsync::ON) ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
				std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eImmediate };
#endif
				swapchain = std::make_unique<Swapchain>(render_device, 
					render_device.GetSurface(), 
					present_mode, 
					present_mode_priority_list, 
					render_device.GetSurfacePriorityList()
				);

				auto image_count = swapchain->GetImageCount();

				// Create synchronization primitives
				image_avaliable_semaphores.resize(image_count);
				render_finished_semaphores.resize(image_count);
				in_flight_fences.resize(image_count);

				vk::SemaphoreCreateInfo ia_semaphore_ci{};
				vk::SemaphoreCreateInfo rf_semaphore_ci{};
				vk::FenceCreateInfo fence_ci{};
				fence_ci.flags = vk::FenceCreateFlagBits::eSignaled;

				for (u64 idx = 0ull; idx < image_count; ++idx)
				{
					image_avaliable_semaphores[idx] = render_device.CreateSemaphore2(ia_semaphore_ci, std::format("image avaliable #{}", idx));
					render_finished_semaphores[idx] = render_device.CreateSemaphore2(rf_semaphore_ci, std::format("render finished #{}", idx));
					in_flight_fences[idx] = render_device.CreateFence2(fence_ci, std::format("frame #{}", idx));
				}
			}

			RenderContext::~RenderContext()
			{
				for (int32_t i = 0; i < swapchain->GetImageCount(); ++i)
				{
					render_device.Destroy(image_avaliable_semaphores[i]);
					render_device.Destroy(render_finished_semaphores[i]);
					render_device.Destroy(in_flight_fences[i]);
				}
			}

			int32_t RenderContext::GetCurrentFrameIndex() const
			{
				return frame_index;
			}

			RenderDevice& RenderContext::GetDevice() const
			{
				return render_device;
			}

			Swapchain& RenderContext::GetSwapchain() const
			{
				return *swapchain;
			}
		}
	}
}