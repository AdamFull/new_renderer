#include "gfx_vk_fence.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"

#include "../../util/Profiler.h"

namespace edge::gfx::vulkan
{
	VulkanFence::VulkanFence(Device& device) :
		VulkanResource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {

		vk::FenceCreateInfo create_info{};
		create_info.flags = vk::FenceCreateFlagBits::eSignaled;

		vk::Result result = device.get_handle().createFence(&create_info, allocation_callbacks, &handle_);
	}

	VulkanFence::~VulkanFence() {
		device.get_handle().destroyFence(handle_, allocation_callbacks);
	}

	void VulkanFence::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	void VulkanFence::wait(uint64_t timeout) {
		EDGE_PROFILE_ZONE_SCOPED;

		vk::Result result = device.get_handle().waitForFences(1u, &handle_, VK_TRUE, timeout);
	}

	void VulkanFence::reset() {
		EDGE_PROFILE_ZONE_SCOPED;

		vk::Result result = device.get_handle().resetFences(1u, &handle_);
	}
}