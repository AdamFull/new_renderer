#pragma once

#include "gfx_vk_queue.h"

// Handles all avaliable queue families and manage it's usage. Need to have methods like: request queue, release queue, etc.

namespace edge::gfx::vulkan
{
	class QueueFamilyManager {
	public:
		QueueFamilyManager();
		QueueFamilyManager(Device& device, uint32_t family_index, vk::QueueFamilyProperties properties, vk::Bool32 can_present);
		QueueFamilyManager(QueueFamilyManager&& other);
		~QueueFamilyManager() = default;

		QueueFamilyManager(const QueueFamilyManager&) = delete;
		QueueFamilyManager& operator=(const QueueFamilyManager&) = delete;
		QueueFamilyManager& operator=(QueueFamilyManager&&) = delete;

		auto get_device() const noexcept -> Device const& {
			return *device;
		}

		auto get_device() noexcept -> Device& {
			return *device;
		}

		[[nodiscard]] auto look_up_queue(QueueTypeFlags include_flags, QueueTypeFlags exclude_flags) noexcept -> VulkanQueue*;

		auto get_family_index() const noexcept -> uint32_t {
			return family_index;
		}

		auto get_flags() const noexcept -> QueueTypeFlags {
			return flags;
		}
	private:
		Device* device{ nullptr };

		uint32_t family_index{ 0 };
		QueueTypeFlags flags{};

		std::vector<VulkanQueue> queues{};
	};
}