#include "VkQueue.h"
#include "VkDevice.h"
#include "VkDebug.h"

#include "VkFence.h"
#include "VkSemaphore.h"
#include "VkSwapchain.h"
#include "VkCommandList.h"

#include "VkUtil.h"

#include "../../util/Profiler.h"

namespace edge::gfx::vulkan
{
	VulkanQueue::VulkanQueue(QueueFamilyManager* family_manager, vk::Queue queue) :
		VulkanResource{ queue, family_manager->get_device().get_allocation_callbacks() },
		family_manager{ family_manager } {
	}

	VulkanQueue::VulkanQueue(VulkanQueue&& other) :
		VulkanResource{ std::move(other) },
		family_manager{ std::exchange(other.family_manager, nullptr) } {
	}

	void VulkanQueue::set_resource_name(std::string_view name) const noexcept {
		auto& device = family_manager->get_device();
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	void VulkanQueue::submit(const SubmitInfo& submit_info, IFence* fence) const {
		EDGE_PROFILE_ZONE_SCOPED;

		std::array<vk::SemaphoreSubmitInfo, 16ull> wait_semaphores{};
		std::array<vk::SemaphoreSubmitInfo, 16ull> signal_semaphores{};
		std::array<vk::CommandBufferSubmitInfo, 16ull> command_buffers{};

		vk::SubmitInfo2KHR vk_submit_info{};
		vk_submit_info.pWaitSemaphoreInfos = wait_semaphores.data();
		vk_submit_info.pSignalSemaphoreInfos = signal_semaphores.data();
		vk_submit_info.pCommandBufferInfos = command_buffers.data();

		for (int32_t i = 0; i < submit_info.wait_semaphore_infos.size(); ++i) {
			SemaphoreSubmitInfo& semaphore_info = submit_info.wait_semaphore_infos[i];
			if (semaphore_info.semaphore) {
				auto& submit_info = wait_semaphores[vk_submit_info.waitSemaphoreInfoCount++];
				submit_info.semaphore = static_cast<VulkanSemaphore*>(semaphore_info.semaphore)->get_handle();
				submit_info.value = semaphore_info.value;
				submit_info.stageMask = stage_flags_to_vk(semaphore_info.stage);
			}
		}

		for (int32_t i = 0; i < submit_info.command_lists.size(); ++i) {
			ICommandList* command_list = submit_info.command_lists[i];
			if (command_list) {
				auto& submit_info = command_buffers[vk_submit_info.commandBufferInfoCount++];
				submit_info.commandBuffer = static_cast<VulkanCommandList*>(command_list)->get_handle();
			}
		}

		for (int32_t i = 0; i < submit_info.signal_semaphore_infos.size(); ++i) {
			SemaphoreSubmitInfo& semaphore_info = submit_info.signal_semaphore_infos[i];
			if (semaphore_info.semaphore) {
				auto& submit_info = signal_semaphores[vk_submit_info.signalSemaphoreInfoCount++];
				submit_info.semaphore = static_cast<VulkanSemaphore*>(semaphore_info.semaphore)->get_handle();
				submit_info.value = semaphore_info.value;
				submit_info.stageMask = stage_flags_to_vk(semaphore_info.stage);
			}
		}

		handle_.submit2KHR(vk_submit_info, fence ? static_cast<VulkanFence*>(fence)->get_handle() : nullptr);
	}

	void VulkanQueue::present(const PresentInfo& present_info) const {
		EDGE_PROFILE_ZONE_SCOPED;

		std::array<vk::Semaphore, 16ull> wait_semaphores{};
		std::array<vk::SwapchainKHR, 16ull> swapchains{};

		vk::PresentInfoKHR vk_present_info{};
		vk_present_info.pWaitSemaphores = wait_semaphores.data();
		vk_present_info.pSwapchains = swapchains.data();
		vk_present_info.pImageIndices = present_info.image_indices;

		for (int32_t i = 0; i < present_info.wait_semaphores.size(); ++i) {
			ISemaphore* semaphore = present_info.wait_semaphores[i];
			if (semaphore) {
				wait_semaphores[vk_present_info.waitSemaphoreCount++] = static_cast<VulkanSemaphore*>(semaphore)->get_handle();
			}
		}

		for (int32_t i = 0; i < present_info.swapchains.size(); ++i) {
			ISwapchain* swapchain = present_info.swapchains[i];
			if (swapchain) {
				auto* vk_swapchain = static_cast<VulkanSwapchain*>(swapchain);
				swapchains[vk_present_info.swapchainCount++] = vk_swapchain->get_handle();

				if (!vk_present_info.pNext) {
					auto& surface_extent = vk_swapchain->get_extent();
					//auto& window = vk_swapchain->GetWidth
					//
					//vk::DisplayPresentInfoKHR disp_present_info;
					//if (device.is_extension_supported(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME) &&
					//	window.GetDisplayPresentInfo(reinterpret_cast<VkDisplayPresentInfoKHR*>(&disp_present_info), surface_extent.width, surface_extent.height))
					//{
					//	// Add display present info if supported and wanted
					//	vk_present_info.pNext = &disp_present_info;
					//}
				}
			}
		}

		handle_.presentKHR(vk_present_info);
	}

	void VulkanQueue::wait_idle() const {
		EDGE_PROFILE_ZONE_SCOPED;

		handle_.waitIdle();
	}
}