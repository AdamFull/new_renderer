#pragma once

#include "VkResource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;

	class VulkanFence final : public IFence, public VulkanResource<vk::Fence> {
	public:
		VulkanFence(Device& device);
		~VulkanFence();

		void set_resource_name(std::string_view name) const noexcept override;

		void wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) override;
		void reset() override;

		VulkanFence(const VulkanFence&) = delete;
		VulkanFence(VulkanFence&& other) = default;
		VulkanFence& operator=(const VulkanFence&) = delete;
		VulkanFence& operator=(VulkanFence&&) = default;
	private:
		Device& device;
	};
}