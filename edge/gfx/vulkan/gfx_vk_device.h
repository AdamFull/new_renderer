#pragma once

#include <unordered_map>

#include "gfx_vk_queue.h"

namespace edge::gfx::vulkan
{
	class Device;
	class DebugUtils;
	class PhysicalDevice;

	struct QueueFamily {
		uint32_t index{ 0 };
		std::vector<VulkanQueue> queues{};
	};

	class Device : public VulkanResource<vk::Device> {
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

		auto get_gpu() const noexcept -> PhysicalDevice const& {
			return gpu;
		}

		auto get_queue_count(QueueType queue_type) -> uint32_t;
		auto get_queue(QueueType queue_type, uint32_t queue_count = 0) -> VulkanQueue*;

		auto get_queue_type_count() const -> size_t {
			return queue_family_count;
		}

		auto is_extension_supported(const char* extension) const noexcept -> bool;

		auto is_enabled(std::string const& extension) const noexcept -> bool;

		auto get_queue_family_index(vk::QueueFlagBits queue_flag) const->uint32_t;

		void set_debug_utils(DebugUtils* debug_utils) noexcept;
		auto get_debug_utils() const noexcept -> DebugUtils const&;

	private:
		DebugUtils* debug_utils{ nullptr };
		PhysicalDevice const& gpu;

		vk::AllocationCallbacks* allocation_callbacks{ nullptr };

		vk::SurfaceKHR surface{ nullptr };

		std::vector<const char*> enabled_extensions{};

		uint32_t queue_family_count{ 0u };
		std::array<std::vector<QueueFamily>, 3ull> queue_families{};
	};
}