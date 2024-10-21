#include <VkSemaphore.h>
#include <VkDevice.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanSemaphore::VulkanSemaphore(Device& device, vk::AllocationCallbacks* allocation_callbacks) :
			VulkanResource{ nullptr, allocation_callbacks },
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