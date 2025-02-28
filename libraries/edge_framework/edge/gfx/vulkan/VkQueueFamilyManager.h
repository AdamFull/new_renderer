#pragma once

#include "VkQueue.h"

// Handles all avaliable queue families and manage it's usage. Need to have methods like: request queue, release queue, etc.

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class QueueFamilyManager
			{
			public:
				QueueFamilyManager();
				QueueFamilyManager(Device& device, uint32_t family_index, vk::QueueFamilyProperties properties, vk::Bool32 can_present);
				QueueFamilyManager(QueueFamilyManager&& other);
				~QueueFamilyManager() = default;

				QueueFamilyManager(const QueueFamilyManager&) = delete;
				QueueFamilyManager& operator=(const QueueFamilyManager&) = delete;
				QueueFamilyManager& operator=(QueueFamilyManager&&) = delete;

				auto GetDevice() const noexcept -> Device const&
				{
					return *device;
				}

				auto GetDevice() noexcept -> Device&
				{
					return *device;
				}

				[[nodiscard]] auto LookUpQueue(QueueTypeFlags include_flags, QueueTypeFlags exclude_flags) noexcept -> VulkanQueue*;

				auto GetFamilyIndex() const noexcept -> uint32_t
				{
					return family_index;
				}

				auto GetFlags() const noexcept -> QueueTypeFlags
				{
					return flags;
				}
			private:
				Device* device{ nullptr };

				uint32_t family_index{ 0 };
				QueueTypeFlags flags{};

				std::vector<VulkanQueue> queues{};
			};
		}
	}
}