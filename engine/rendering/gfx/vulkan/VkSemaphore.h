#pragma once

#include "VkResource.h"
#include <Base.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;

			class VulkanSemaphore : public Semaphore, public VulkanResource<vk::Semaphore>
			{
			public:
				VulkanSemaphore(Device& device);
				~VulkanSemaphore();

				VulkanSemaphore(const VulkanSemaphore&) = delete;
				VulkanSemaphore(VulkanSemaphore&& other) = default;
				VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;
				VulkanSemaphore& operator=(VulkanSemaphore&&) = default;

				void SetResourceName(const char* name) override;
			private:
				Device& device;
			};
		}
	}
}