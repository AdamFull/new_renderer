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
			RenderContext::RenderContext(Window* window, VkRenderDevice& render_device) :
				device{ render_device.GetLogicalDevice() }
			{
#ifdef VK_USE_PLATFORM_ANDROID_KHR
				vk::PresentModeKHR present_mode = (window->GetProperties().vsync == Window::Vsync::OFF) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
				std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate };
#else
				vk::PresentModeKHR present_mode = (window->GetProperties().vsync == Window::Vsync::ON) ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
				std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eImmediate };
#endif
				swapchain = std::make_unique<Swapchain>(render_device.GetLogicalDevice(), 
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

				FVkSemaphoreCreateInfo ia_semaphore_ci{};
				FVkSemaphoreCreateInfo rf_semaphore_ci{};
				FVkFenceCreateInfo fence_ci{};
				fence_ci.ci.flags = vk::FenceCreateFlagBits::eSignaled;
				for (u64 idx = 0ull; idx < image_count; ++idx)
				{
#ifndef BUILD_WITHOUT_DEBUGGIND
					ia_semaphore_ci.name = std::format("image avaliable #{}", idx);
					rf_semaphore_ci.name = std::format("render finished #{}", idx);
					fence_ci.name = std::format("frame #{}", idx);
#endif
					render_device.CreateSemaphore(ia_semaphore_ci, image_avaliable_semaphores[idx]);
					render_device.CreateSemaphore(rf_semaphore_ci, render_finished_semaphores[idx]);
					render_device.CreateFence(fence_ci, in_flight_fences[idx]);
				}
			}

			RenderContext::~RenderContext()
			{
				for (int32_t i = 0; i < swapchain->GetImageCount(); ++i)
				{
					device.DestroyObject(image_avaliable_semaphores[i]);
					device.DestroyObject(render_finished_semaphores[i]);
					device.DestroyObject(in_flight_fences[i]);
				}
			}

			int32_t RenderContext::GetCurrentFrameIndex() const
			{
				return frame_index;
			}

			Device& RenderContext::GetDevice() const
			{
				return device;
			}

			Swapchain& RenderContext::GetSwapchain() const
			{
				return *swapchain;
			}
		}
	}
}