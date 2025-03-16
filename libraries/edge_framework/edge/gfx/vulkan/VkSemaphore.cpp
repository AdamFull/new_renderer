#include "VkSemaphore.h"
#include "VkDevice.h"
#include "VkDebug.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanSemaphore::VulkanSemaphore(Device& device, bool timeline) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device }
			{
				vk::SemaphoreTypeCreateInfo timeline_semaphore_ci{};
				timeline_semaphore_ci.semaphoreType = timeline ? vk::SemaphoreType::eTimeline : vk::SemaphoreType::eBinary;
				timeline_semaphore_ci.initialValue = 0;

				vk::SemaphoreCreateInfo create_info{};
				create_info.pNext = &timeline_semaphore_ci;
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