#include "VkFence.h"
#include "VkDevice.h"

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanFence::VulkanFence(Device& device) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device }
			{
				vk::FenceCreateInfo create_info{};
				create_info.flags = vk::FenceCreateFlagBits::eSignaled;

				vk::Result result = Handle(device).createFence(&create_info, allocation_callbacks, &handle);
			}

			VulkanFence::~VulkanFence()
			{
				Handle(device).destroyFence(handle, allocation_callbacks);
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
}