#include "VkSemaphore.h"
#include "VkDevice.h"
#include "VkDebug.h"

namespace edge
{
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

			void VulkanSemaphore::SetResourceName(std::string_view name) const noexcept
			{
				auto& debug_utils = device.GetDebugUtils();
				debug_utils.SetDebugName(handle, name);
			}
		}
	}
}