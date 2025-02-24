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

				Device const& GetDevice() const;
				Device& GetDevice();

				VulkanQueue const& GetQueue(uint32_t index) const;

				bool LookAtQueue(QueueTypeFlags include_flags, QueueTypeFlags exclude_flags, uint32_t& queue_index) const;

				uint32_t GetFamilyIndex() const;
				QueueTypeFlags GetFlags() const;
			private:
				Device* device{ nullptr };

				uint32_t family_index{ 0 };
				QueueTypeFlags flags{};

				std::vector<VulkanQueue> queues{};
			};
		}
	}
}