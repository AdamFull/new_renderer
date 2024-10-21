#pragma once

#include <VkResource.h>
#include <GFX.h>

namespace gfx
{
	namespace vulkan
	{
		class Device;

		class VulkanSemaphore : public Semaphore, public VulkanResource<vk::Semaphore>
		{
		public:
			VulkanSemaphore(Device& device, vk::AllocationCallbacks* allocation_callbacks);
			~VulkanSemaphore();

			VulkanSemaphore(const VulkanSemaphore&) = delete;
			VulkanSemaphore(VulkanSemaphore&& other) = default;
			VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;
			VulkanSemaphore& operator=(VulkanSemaphore&&) = default;
		private:
			Device& device;
		};
	}
}