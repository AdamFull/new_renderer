#include "gfx_vk_queue_family_manager.h"
#include "gfx_vk_device.h"

namespace edge::gfx::vulkan
{
	QueueFamilyManager::QueueFamilyManager() {
	}

	QueueFamilyManager::QueueFamilyManager(Device& device, uint32_t family_index, vk::QueueFamilyProperties queue_family_property, vk::Bool32 can_present) :
		device{ &device },
		family_index{ family_index } {
		// Create queues
		for (uint32_t queue_index = 0U; queue_index < queue_family_property.queueCount; ++queue_index) {
			queues.emplace_back(this, device.get_handle().getQueue(family_index, queue_index));
		}

		if (can_present)
			flags |= QueueTypeFlag::ePresent;
		if (queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics)
			flags |= QueueTypeFlag::eGraphics;
		if (queue_family_property.queueFlags & vk::QueueFlagBits::eCompute)
			flags |= QueueTypeFlag::eCompute;
		if (queue_family_property.queueFlags & vk::QueueFlagBits::eTransfer)
			flags |= QueueTypeFlag::eTransfer;
	}

	QueueFamilyManager::QueueFamilyManager(QueueFamilyManager&& other) :
		device(other.device),
		family_index{ std::exchange(other.family_index, {}) },
		flags{ std::exchange(other.flags, {}) },
		queues{ std::exchange(other.queues, {}) } {

		for (auto& queue : queues) {
			queue.set_family(this);
		}
	}

	auto QueueFamilyManager::look_up_queue(QueueTypeFlags include_flags, QueueTypeFlags exclude_flags) noexcept -> VulkanQueue* {
		bool is_contain_include_flags = (flags & include_flags) == include_flags;
		bool is_contain_exclude_flags = (flags & exclude_flags) != 0;

		if (is_contain_include_flags && !is_contain_exclude_flags) {
			for (auto& queue : queues) {
				if (!queue.is_used()) {
					queue.set_used(true);
					return &queue;
				}
			}
		}

		return nullptr;
	}
}