#pragma once

#include <unordered_map>

#include "VkQueueFamilyManager.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;
			class DebugUtils;
			class PhysicalDevice;

			class Device : public VulkanResource<vk::Device>
			{
			public:
				/**
				 * @brief Device constructor
				 * @param gpu A valid Vulkan physical device and the requested gpu features
				 * @param surface The surface
				 * @param debug_utils The debug utils to be associated to this device
				 * @param requested_extensions (Optional) List of required device extensions and whether support is optional or not
				 */
				Device(PhysicalDevice& gpu, vk::SurfaceKHR surface, std::vector<const char*> requested_extensions = {});

				Device(const Device&) = delete;
				Device(Device&&) = delete;

				~Device();

				Device& operator=(const Device&) = delete;
				Device& operator=(Device&&) = delete;

				auto GetGpu() const noexcept -> PhysicalDevice const&
				{
					return gpu;
				}

				[[nodiscard]] auto RequestQueue(QueueTypeFlags include_flags, QueueTypeFlags exclude_flags) -> VulkanQueue*;

				auto GetQueueCount() const -> size_t
				{
					return queue_families.size();
				}

				auto IsExtensionSupported(const char* extension) const noexcept -> bool;

				auto IsEnabled(std::string const& extension) const noexcept -> bool;

				auto GetQueueFamilyIndex(vk::QueueFlagBits queue_flag) const -> uint32_t;

				void SetDebugUtils(DebugUtils* debug_utils) noexcept;
				auto GetDebugUtils() const noexcept -> DebugUtils const&;

			private:
				DebugUtils* debug_utils{ nullptr };
				PhysicalDevice const& gpu;

				vk::AllocationCallbacks* allocation_callbacks{ nullptr };

				vk::SurfaceKHR surface{ nullptr };

				std::vector<const char*> enabled_extensions{};

				std::vector<QueueFamilyManager> queue_families{};
			};
		}
	}
}