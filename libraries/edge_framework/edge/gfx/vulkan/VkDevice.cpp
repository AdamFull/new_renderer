#include "VkDevice.h"
#include "VkPhysicalDevice.h"

#include "../../util/Logger.h"

#include <volk.h>

namespace edge::gfx::vulkan
{
	Device::Device(PhysicalDevice& gpu, vk::SurfaceKHR surface, std::vector<const char*> requested_extensions) :
		VulkanResource{ nullptr, gpu.get_allocation_callbacks() },
		gpu(gpu) {
		// Prepare the device queues
		auto& queue_family_properties = gpu.get_queue_family_properties();
		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos(queue_family_properties.size());
		std::vector<std::vector<float>> queue_priorities(queue_family_properties.size());

		for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_properties.size(); ++queue_family_index) {
			vk::QueueFamilyProperties const& queue_family_property = queue_family_properties[queue_family_index];

			if (gpu.has_high_priority_graphics_queue()) {
				uint32_t graphics_queue_family = get_queue_family_index(vk::QueueFlagBits::eGraphics);
				if (graphics_queue_family == queue_family_index) {
					queue_priorities[queue_family_index].reserve(queue_family_property.queueCount);
					queue_priorities[queue_family_index].emplace_back(1.0f);
					for (uint32_t i = 1; i < queue_family_property.queueCount; i++) {
						queue_priorities[queue_family_index].emplace_back(0.5f);
					}
				}
				else {
					queue_priorities[queue_family_index].resize(queue_family_property.queueCount, 0.5f);
				}
			}
			else {
				queue_priorities[queue_family_index].resize(queue_family_property.queueCount, 0.5f);
			}

			vk::DeviceQueueCreateInfo& queue_create_info = queue_create_infos[queue_family_index];

			queue_create_info.queueFamilyIndex = queue_family_index;
			queue_create_info.queueCount = queue_family_property.queueCount;
			queue_create_info.pQueuePriorities = queue_priorities[queue_family_index].data();
		}

		// Check extensions to enable Vma Dedicated Allocation
		bool can_get_memory_requirements = is_extension_supported(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
		bool has_dedicated_allocation = is_extension_supported(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);

		if (can_get_memory_requirements && has_dedicated_allocation) {
			enabled_extensions.emplace_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
			enabled_extensions.emplace_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);

			EDGE_LOGI("[VkDevice] Dedicated Allocation enabled");
		}

		// For performance queries, we also use host query reset since queryPool resets cannot
		// live in the same command buffer as beginQuery
		if (is_extension_supported(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME) && is_extension_supported(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
			auto perf_counter_features = gpu.request_extension_features<vk::PhysicalDevicePerformanceQueryFeaturesKHR>();
			auto host_query_reset_features = gpu.request_extension_features<vk::PhysicalDeviceHostQueryResetFeatures>();

			if (perf_counter_features.performanceCounterQueryPools && host_query_reset_features.hostQueryReset) {
				enabled_extensions.emplace_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
				enabled_extensions.emplace_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
				EDGE_LOGI("[VkDevice] Performance query enabled");
			}
		}

		enabled_extensions.insert(enabled_extensions.end(), requested_extensions.begin(), requested_extensions.end());

		if (enabled_extensions.size() > 0) {
			EDGE_LOGI("[VkDevice] Device supports the following requested extensions:");
			for (auto& extension : enabled_extensions) {
				EDGE_LOGI("  \t{}", extension);
			}
		}

		vk::DeviceCreateInfo create_info({}, queue_create_infos, {}, enabled_extensions, &gpu.get_mutable_requested_features());

#if USE_NSIGHT_AFTERMATH
		// Initialize Nsight Aftermath for this device.
		//
		// * ENABLE_RESOURCE_TRACKING - this will include additional information about the
		//   resource related to a GPU virtual address seen in case of a crash due to a GPU
		//   page fault. This includes, for example, information about the size of the
		//   resource, its format, and an indication if the resource has been deleted.
		//
		// * ENABLE_AUTOMATIC_CHECKPOINTS - this will enable automatic checkpoints for
		//   all draw calls, compute dispatchs, and resource copy operations that capture
		//   CPU call stacks for those event.
		//
		//   Using this option should be considered carefully. It can cause very high CPU overhead.
		//
		// * ENABLE_SHADER_DEBUG_INFO - this instructs the shader compiler to
		//   generate debug information (line tables) for all shaders. Using this option
		//   should be considered carefully. It may cause considerable shader compilation
		//   overhead and additional overhead for handling the corresponding shader debug
		//   information callbacks.
		//

		vk::DeviceDiagnosticsConfigCreateInfoNV aftermath_info{};
		aftermath_info.flags =
			vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableResourceTracking |
			vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableAutomaticCheckpoints |
			vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableShaderDebugInfo;

		if (is_enabled(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME))
		{
			aftermath_info.pNext = gpu.GetExtensionFeatureChain();
			create_info.pNext = &aftermath_info;
		}
		else
		{
			create_info.pNext = gpu.GetExtensionFeatureChain();
		}
#else
		// Latest requested feature will have the pNext's all set up for device creation.
		create_info.pNext = gpu.get_extension_feature_chain();
#endif

		handle_ = gpu.get_handle().createDevice(create_info);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(handle_);

		// Need to load volk for all the not-yet Vulkan-Hpp calls
		volkLoadDevice(handle_);

		for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_properties.size(); ++queue_family_index) {
			vk::QueueFamilyProperties const& queue_family_property = queue_family_properties[queue_family_index];
			vk::Bool32 present_supported = gpu.get_handle().getSurfaceSupportKHR(queue_family_index, surface);
			queue_families.emplace_back(*this, queue_family_index, queue_family_property, present_supported);
		}
	}

	Device::~Device() {
		if (handle_)
			handle_.destroy();
	}

	auto Device::request_queue(QueueTypeFlags include_flags, QueueTypeFlags exclude_flags) -> VulkanQueue* {
		for (auto& family : queue_families) {
			if (auto* queue = family.look_up_queue(include_flags, exclude_flags))
				return queue;
		}

		return nullptr;
	}

	auto Device::is_extension_supported(const char* extension) const noexcept -> bool {
		return gpu.is_extension_supported(extension);
	}

	auto Device::is_enabled(std::string const& extension) const noexcept -> bool {
		return std::find_if(enabled_extensions.begin(), enabled_extensions.end(),
			[extension](const char* enabled_extension) { return extension == enabled_extension; }) != enabled_extensions.end();
	}

	auto Device::get_queue_family_index(vk::QueueFlagBits queue_flag) const -> uint32_t {
		const auto& queue_family_properties = gpu.get_queue_family_properties();

		// Dedicated queue for compute
		// Try to find a queue family index that supports compute but not graphics
		if (queue_flag & vk::QueueFlagBits::eCompute) {
			for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++) {
				if ((queue_family_properties[i].queueFlags & queue_flag) && !(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
					return i;
					break;
				}
			}
		}

		// Dedicated queue for transfer
		// Try to find a queue family index that supports transfer but not graphics and compute
		if (queue_flag & vk::QueueFlagBits::eTransfer) {
			for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++) {
				if ((queue_family_properties[i].queueFlags & queue_flag) && !(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
					!(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eCompute)) {
					return i;
					break;
				}
			}
		}

		// For other queue types or if no separate compute queue is present, return the first one to support the requested
		// flags
		for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++) {
			if (queue_family_properties[i].queueFlags & queue_flag) {
				return i;
				break;
			}
		}

		EDGE_LOGE("[VkDevice] Could not find a matching queue family index");
	}

	void Device::set_debug_utils(DebugUtils* debug_utils) noexcept {
		this->debug_utils = debug_utils;
	}

	auto Device::get_debug_utils() const noexcept -> DebugUtils const& {
		return *debug_utils;
	}
}