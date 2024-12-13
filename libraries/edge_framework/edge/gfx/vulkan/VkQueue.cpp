#include "VkQueue.h"
#include "VkDevice.h"
#include "VkDebug.h"

#include "VkFence.h"
#include "VkSemaphore.h"
#include "VkSwapchain.h"
#include "VkCommandList.h"

#include "VkUtil.h"

#include <tracy/Tracy.hpp>

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanQueue::VulkanQueue(Device& device, uint32_t family_index, vk::QueueFamilyProperties properties, vk::Bool32 can_present, uint32_t index) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device },
				family_index{ family_index },
				index{ index },
				can_present{ can_present },
				properties{ properties }
			{
				handle = Handle(device).getQueue(family_index, index);
			}

			VulkanQueue::VulkanQueue(VulkanQueue&& other) :
				VulkanResource{ std::move(other) },
				device(other.device),
				family_index(std::exchange(other.family_index, {})),
				index(std::exchange(other.index, 0)),
				can_present(std::exchange(other.can_present, false)),
				properties(std::exchange(other.properties, {}))
			{
			}

			const Device& VulkanQueue::GetDevice() const
			{
				return device;
			}

			Device& VulkanQueue::GetDevice()
			{
				return device;
			}

			uint32_t VulkanQueue::GetFamilyIndex() const
			{
				return family_index;
			}

			uint32_t VulkanQueue::GetIndex() const
			{
				return index;
			}

			const vk::QueueFamilyProperties& VulkanQueue::GetProperties() const
			{
				return properties;
			}

			vk::Bool32 VulkanQueue::SupportPresent() const
			{
				return can_present;
			}

			void VulkanQueue::SetResourceName(std::string_view name)
			{
				auto& debug_utils = device.GetDebugUtils();
				debug_utils.SetDebugName(handle, name);
			}

			void VulkanQueue::Submit(const SubmitInfo& submit_info, Fence* fence) const
			{
				ZoneScoped;

				std::array<vk::SemaphoreSubmitInfo, 16ull> wait_semaphores{};
				std::array<vk::SemaphoreSubmitInfo, 16ull> signal_semaphores{};
				std::array<vk::CommandBufferSubmitInfo, 16ull> command_buffers{};

				vk::SubmitInfo2KHR vk_submit_info{};
				vk_submit_info.pWaitSemaphoreInfos = wait_semaphores.data();
				vk_submit_info.pSignalSemaphoreInfos = signal_semaphores.data();
				vk_submit_info.pCommandBufferInfos = command_buffers.data();

				for (int32_t i = 0; i < submit_info.wait_semaphore_count; ++i)
				{
					SemaphoreSubmitInfo& semaphore_info = submit_info.wait_semaphore_infos[i];
					if (semaphore_info.semaphore)
					{
						auto& submit_info = wait_semaphores[vk_submit_info.waitSemaphoreInfoCount++];
						submit_info.semaphore = static_cast<VulkanSemaphore*>(semaphore_info.semaphore)->GetHandle();
						submit_info.value = semaphore_info.value;
						submit_info.stageMask = StageFlagsToVk(semaphore_info.stage);
					}
				}

				for (int32_t i = 0; i < submit_info.command_buffer_count; ++i)
				{
					CommandList* command_list = submit_info.command_lists[i];
					if (command_list)
					{
						auto& submit_info = command_buffers[vk_submit_info.commandBufferInfoCount++];
						submit_info.commandBuffer = static_cast<VulkanCommandList*>(command_list)->GetHandle();
					}
				}

				for (int32_t i = 0; i < submit_info.signal_semaphore_count; ++i)
				{
					SemaphoreSubmitInfo& semaphore_info = submit_info.signal_semaphore_infos[i];
					if (semaphore_info.semaphore)
					{
						auto& submit_info = signal_semaphores[vk_submit_info.signalSemaphoreInfoCount++];
						submit_info.semaphore = static_cast<VulkanSemaphore*>(semaphore_info.semaphore)->GetHandle();
						submit_info.value = semaphore_info.value;
						submit_info.stageMask = StageFlagsToVk(semaphore_info.stage);
					}
				}

				handle.submit2KHR(vk_submit_info, fence ? static_cast<VulkanFence*>(fence)->GetHandle() : nullptr);
			}

			void VulkanQueue::Present(const PresentInfo& present_info) const
			{
				ZoneScoped;

				std::array<vk::Semaphore, 16ull> wait_semaphores{};
				std::array<vk::SwapchainKHR, 16ull> swapchains{};

				vk::PresentInfoKHR vk_present_info{};
				vk_present_info.pWaitSemaphores = wait_semaphores.data();
				vk_present_info.pSwapchains = swapchains.data();
				vk_present_info.pImageIndices = present_info.image_indices;

				for (int32_t i = 0; i < present_info.wait_semaphore_count; ++i)
				{
					Semaphore* semaphore = present_info.wait_semaphores[i];
					if (semaphore)
					{
						wait_semaphores[vk_present_info.waitSemaphoreCount++] = static_cast<VulkanSemaphore*>(semaphore)->GetHandle();
					}
				}

				for (int32_t i = 0; i < present_info.swapchain_count; ++i)
				{
					Swapchain* swapchain = present_info.swapchains[i];
					if (swapchain)
					{
						auto* vk_swapchain = static_cast<VulkanSwapchain*>(swapchain);
						swapchains[vk_present_info.swapchainCount++] = vk_swapchain->GetHandle();

						if (!vk_present_info.pNext)
						{
							auto& surface_extent = vk_swapchain->GetExtent();
							//auto& window = vk_swapchain->GetWidth
							//
							//vk::DisplayPresentInfoKHR disp_present_info;
							//if (device.IsExtensionSupported(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME) &&
							//	window.GetDisplayPresentInfo(reinterpret_cast<VkDisplayPresentInfoKHR*>(&disp_present_info), surface_extent.width, surface_extent.height))
							//{
							//	// Add display present info if supported and wanted
							//	vk_present_info.pNext = &disp_present_info;
							//}
						}
					}
				}

				handle.presentKHR(vk_present_info);
			}

			void VulkanQueue::WaitIdle() const
			{
				ZoneScoped;

				handle.waitIdle();
			}
		}
	}
}