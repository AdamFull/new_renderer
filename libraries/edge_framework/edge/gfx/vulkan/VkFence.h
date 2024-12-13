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

			class VulkanFence : public Fence, public VulkanResource<vk::Fence>
			{
			public:
				VulkanFence(Device& device);
				~VulkanFence();

				void SetResourceName(std::string_view name) override;

				void Wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) override;
				void Reset() override;

				VulkanFence(const VulkanFence&) = delete;
				VulkanFence(VulkanFence&& other) = default;
				VulkanFence& operator=(const VulkanFence&) = delete;
				VulkanFence& operator=(VulkanFence&&) = default;
			private:
				Device& device;
			};
		}
	}
}