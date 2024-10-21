#include <VkSampler.h>
#include <VkDevice.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanSampler::VulkanSampler(Device& device, vk::AllocationCallbacks* allocation_callbacks, const vk::SamplerCreateInfo& create_info) :
			VulkanResource{ nullptr, allocation_callbacks },
			device{ device }
		{
			vk::Result result = Handle(device).createSampler(&create_info, allocation_callbacks, &handle);
		}

		VulkanSampler::VulkanSampler(VulkanSampler&& other) noexcept :
			VulkanResource{ std::move(other) },
			device{ other.device }
		{
		}

		VulkanSampler::~VulkanSampler()
		{
			Handle(device).destroySampler(handle, allocation_callbacks);
		}
	}
}