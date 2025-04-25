#include "gfx_vk_queue.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"

#include "gfx_vk_fence.h"
#include "gfx_vk_semaphore.h"
#include "gfx_vk_swapchain.h"
#include "gfx_vk_command_list.h"

#include "gfx_vk_util.h"

#include "../../util/utl_profiler.h"

namespace edge::gfx::vulkan
{
	Queue::Queue(Device& device, uint32_t family_index, uint32_t queue_index) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device },
		family_index{ family_index },
		queue_index{ queue_index } {

		vk::DeviceQueueInfo2 queue_info{};
		queue_info.queueFamilyIndex = family_index;
		queue_info.queueIndex = queue_index;

		device.get_handle().getQueue2(&queue_info, &handle_);
	}

	Queue::Queue(Queue&& other) :
		Resource{ std::move(other) },
		device{ other.device },
		family_index{ std::exchange(other.family_index, {}) },
		queue_index{ std::exchange(other.queue_index, {}) } {
	}

	void Queue::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	void Queue::submit(const SubmitInfo& submit_info, IFence* fence) const {
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
				submit_info.semaphore = utl::cast<Semaphore*>(semaphore_info.semaphore)->get_handle();
				submit_info.value = semaphore_info.value;
				submit_info.stageMask = stage_flags_to_vk(semaphore_info.stage);
			}
		}

		for (int32_t i = 0; i < submit_info.command_lists.size(); ++i) {
			ICommandList* command_list = submit_info.command_lists[i];
			if (command_list) {
				auto& submit_info = command_buffers[vk_submit_info.commandBufferInfoCount++];
				submit_info.commandBuffer = utl::cast<CommandList*>(command_list)->get_handle();
			}
		}

		for (int32_t i = 0; i < submit_info.signal_semaphore_infos.size(); ++i) {
			SemaphoreSubmitInfo& semaphore_info = submit_info.signal_semaphore_infos[i];
			if (semaphore_info.semaphore) {
				auto& submit_info = signal_semaphores[vk_submit_info.signalSemaphoreInfoCount++];
				submit_info.semaphore = utl::cast<Semaphore*>(semaphore_info.semaphore)->get_handle();
				submit_info.value = semaphore_info.value;
				submit_info.stageMask = stage_flags_to_vk(semaphore_info.stage);
			}
		}

		handle_.submit2KHR(vk_submit_info, fence ? utl::cast<Fence*>(fence)->get_handle() : nullptr);
	}

	void Queue::present(const PresentInfo& present_info) const {
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
				wait_semaphores[vk_present_info.waitSemaphoreCount++] = utl::cast<Semaphore*>(semaphore)->get_handle();
			}
		}

		for (int32_t i = 0; i < present_info.swapchains.size(); ++i) {
			ISwapchain* swapchain = present_info.swapchains[i];
			if (swapchain) {
				auto* vk_swapchain = utl::cast<Swapchain*>(swapchain);
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

		auto result = handle_.presentKHR(vk_present_info);
	}

	void Queue::wait_idle() const {
		EDGE_PROFILE_ZONE_SCOPED;

		handle_.waitIdle();
	}
}