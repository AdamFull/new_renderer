#pragma once

#include "VkResource.h"
#include "../gfx_base.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;

			class VulkanSampler final : public ISampler, public VulkanResource<vk::Sampler>
			{
			public:
				VulkanSampler(Device& device, const SamplerCreateInfo& create_info);
				~VulkanSampler();

				VulkanSampler(VulkanSampler&) = delete;
				VulkanSampler(VulkanSampler&& other) noexcept;

				VulkanSampler& operator=(const VulkanSampler&) = delete;
				VulkanSampler& operator=(VulkanSampler&&) = delete;

				void SetResourceName(std::string_view name) const noexcept override;
			private:
				Device& device;
			};
		}
	}
}