#include <VkQueue.h>
#include <VkDevice.h>

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
			vk::SubmitInfo2KHR vk_submit_info{};

			//handle.submit(vk_submit_info, fence);
		}

		void VulkanQueue::Present(const PresentInfo& present_info)
		{
			vk::PresentInfoKHR vk_present_info{};


			handle.presentKHR(vk_present_info);
		}

		void VulkanQueue::WaitIdle()
		{
			handle.waitIdle();
		}
	}
}