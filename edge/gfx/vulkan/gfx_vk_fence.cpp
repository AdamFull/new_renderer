#include "gfx_vk_fence.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"

#include "../../util/utl_profiler.h"

namespace edge::gfx::vulkan
{
	Fence::Fence(Device& device) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {

		vk::FenceCreateInfo create_info{};
		create_info.flags = vk::FenceCreateFlagBits::eSignaled;

		vk::Result result = device.get_handle().createFence(&create_info, allocation_callbacks, &handle_);
	}

	Fence::~Fence() {
		device.get_handle().destroyFence(handle_, allocation_callbacks);
	}

	void Fence::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	void Fence::wait(uint64_t timeout) {
		EDGE_PROFILE_ZONE_SCOPED;

		vk::Result result = device.get_handle().waitForFences(1u, &handle_, VK_TRUE, timeout);
	}

	void Fence::reset() {
		EDGE_PROFILE_ZONE_SCOPED;

		vk::Result result = device.get_handle().resetFences(1u, &handle_);
	}
}