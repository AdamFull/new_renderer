#include "gfx_vk_semaphore.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"

namespace edge::gfx::vulkan
{
	Semaphore::Semaphore(Device& device, bool timeline) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {

		vk::SemaphoreTypeCreateInfo timeline_semaphore_ci{};
		timeline_semaphore_ci.semaphoreType = timeline ? vk::SemaphoreType::eTimeline : vk::SemaphoreType::eBinary;
		timeline_semaphore_ci.initialValue = 0;

		vk::SemaphoreCreateInfo create_info{};
		create_info.pNext = &timeline_semaphore_ci;
		vk::Result result = device.get_handle().createSemaphore(&create_info, allocation_callbacks, &handle_);
	}

	Semaphore::~Semaphore() {
		device.get_handle().destroySemaphore(handle_, allocation_callbacks);
	}

	void Semaphore::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}
}