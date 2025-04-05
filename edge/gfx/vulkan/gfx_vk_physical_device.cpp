#include "gfx_vk_physical_device.h"

#include "../../util/Logger.h"
#include <vulkan/vulkan_to_string.hpp>

#include <iomanip>

namespace edge::gfx::vulkan
{
	PhysicalDevice::PhysicalDevice(Instance& instance, vk::PhysicalDevice physical_device) :
		instance{ instance },
		Resource{ physical_device, instance.get_allocation_callbacks() } {

		features = physical_device.getFeatures();
		properties = physical_device.getProperties();
		memory_properties = physical_device.getMemoryProperties();

		std::stringstream uuid_ss;
		for (uint8_t byte : properties.pipelineCacheUUID)
			uuid_ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);

		EDGE_LOGI("[VkPhysicalDevice] Found GPU: {}", properties.deviceName.data());
		EDGE_LOGT("  \tDevice ID: {}", properties.deviceID);
		EDGE_LOGT("  \tVendor ID: {}", properties.vendorID);
		EDGE_LOGT("  \tDriver Version: {}", properties.driverVersion);
		EDGE_LOGT("  \tAPI Version: {}", properties.apiVersion);
		EDGE_LOGT("  \tDevice Type: {}", vk::to_string(properties.deviceType));
		EDGE_LOGT("  \tPipeline Cache UUID: {}", uuid_ss.str());

		queue_family_properties = physical_device.getQueueFamilyProperties();

		device_extensions = physical_device.enumerateDeviceExtensionProperties();

		// Display supported extensions
		if (device_extensions.size() > 0) {
			EDGE_LOGD("[VkPhysicalDevice] Device supports the following extensions:");
			for (auto& extension : device_extensions) {
				EDGE_LOGD("  \t{}", extension.extensionName.data());
			}
		}
	}

	auto PhysicalDevice::get_driver_version() const noexcept -> DriverVersion {
		DriverVersion version;

		vk::PhysicalDeviceProperties const& properties = get_properties();
		switch (properties.vendorID) {
		case 0x10DE:
			// Nvidia
			version.major = (properties.driverVersion >> 22) & 0x3ff;
			version.minor = (properties.driverVersion >> 14) & 0x0ff;
			version.patch = (properties.driverVersion >> 6) & 0x0ff;
			// Ignoring optional tertiary info in lower 6 bits
			break;
		case 0x8086:
			version.major = (properties.driverVersion >> 14) & 0x3ffff;
			version.minor = properties.driverVersion & 0x3ffff;
			break;
		default:
			version.major = VK_VERSION_MAJOR(properties.driverVersion);
			version.minor = VK_VERSION_MINOR(properties.driverVersion);
			version.patch = VK_VERSION_PATCH(properties.driverVersion);
			break;
		}

		return version;
	}

	auto PhysicalDevice::is_extension_supported(const char* requested_extension) const noexcept -> bool {
		return std::find_if(device_extensions.begin(), device_extensions.end(),
			[requested_extension](auto& device_extension) { return std::strcmp(device_extension.extensionName, requested_extension) == 0; }) != device_extensions.end();
	}

	auto PhysicalDevice::get_memory_type(uint32_t bits, vk::MemoryPropertyFlags properties, vk::Bool32* memory_type_found) const noexcept -> uint32_t {

		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
			if ((bits & 1) == 1) {
				if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
					if (memory_type_found) {
						*memory_type_found = true;
					}
					return i;
				}
			}
			bits >>= 1;
		}

		if (memory_type_found) {
			*memory_type_found = false;
			return ~0;
		}
		else {
			EDGE_LOGE("[VkPhysicalDevice] Could not find a matching memory type");
		}
	}
}