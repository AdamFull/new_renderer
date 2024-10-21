#include <VkQueue.h>
#include <VkDevice.h>

#include <VkFence.h>
#include <VkSemaphore.h>

#include <VkUtil.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanQueue::VulkanQueue(Device& device, uint32_t family_index, vk::QueueFamilyProperties properties, vk::Bool32 can_present, uint32_t index) :
			VulkanResource{ nullptr, nullptr },
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

		void VulkanQueue::Submit(const SubmitInfo& submit_info, Fence* fence)
		{
			assert(false && "NOT IMPLEMENTED");

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
					//submit_info.commandBuffer = static_cast<VulkanCommandList*>(command_list)->GetHandle();
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

		void VulkanQueue::Present(const PresentInfo& present_info)
		{
			assert(false && "NOT IMPLEMENTED");
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
					//swapchains[vk_present_info.swapchainCount++] = static_cast<VulkanSwapchain*>(swapchain)->GetHandle();
				}
			}

			handle.presentKHR(vk_present_info);
		}

		void VulkanQueue::WaitIdle()
		{
			handle.waitIdle();
		}
	}
}