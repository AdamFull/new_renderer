#include <VkFence.h>
#include <VkDevice.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanFence::VulkanFence(Device& device, vk::AllocationCallbacks* allocation_callbacks) :
			VulkanResource{ nullptr, allocation_callbacks },
			device{ device }
		{
			vk::FenceCreateInfo create_info{};
			create_info.flags = vk::FenceCreateFlagBits::eSignaled;

			vk::Result result = Handle(device).createFence(&create_info, allocation_callbacks, &handle);
		}

		VulkanFence::~VulkanFence()
		{
			Handle(device).destroyFence(handle);
		}

		void VulkanFence::Wait(uint64_t timeout)
		{
			vk::Result result = Handle(device).waitForFences(1u, &handle, VK_TRUE, timeout);
		}

		void VulkanFence::Reset()
		{
			vk::Result result = Handle(device).resetFences(1u, &handle);
		}
	}
}