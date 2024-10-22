#include <VkSemaphore.h>
#include <VkDevice.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanSemaphore::VulkanSemaphore(Device& device) :
			VulkanResource{ nullptr, device.GetAllocationCallbacks() },
			device{ device }
		{
			vk::SemaphoreCreateInfo create_info{};
			vk::Result result = Handle(device).createSemaphore(&create_info, allocation_callbacks, &handle);
		}

		VulkanSemaphore::~VulkanSemaphore()
		{
			Handle(device).destroySemaphore(handle, allocation_callbacks);
		}
	}
}