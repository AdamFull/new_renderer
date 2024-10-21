#pragma once

#include <VkResource.h>
#include <GFX.h>

namespace gfx
{
	namespace vulkan
	{
		class Device;

		class VulkanSampler : public Sampler, public VulkanResource<vk::Sampler>
		{
		public:
			VulkanSampler(Device& device, vk::AllocationCallbacks* allocation_callbacks, const vk::SamplerCreateInfo& create_info);
			~VulkanSampler();

			VulkanSampler(VulkanSampler&) = delete;
			VulkanSampler(VulkanSampler&& other) noexcept;

			VulkanSampler& operator=(const VulkanSampler&) = delete;
			VulkanSampler& operator=(VulkanSampler&&) = delete;
		private:
			Device& device;
		};
	}
}