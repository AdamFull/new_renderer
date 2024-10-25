#pragma once

#include <VkResource.h>
#include <Base.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;

			class VulkanSampler : public Sampler, public VulkanResource<vk::Sampler>
			{
			public:
				VulkanSampler(Device& device, const SamplerCreateInfo& create_info);
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
}