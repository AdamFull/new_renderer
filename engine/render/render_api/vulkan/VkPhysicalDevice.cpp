#include <VkPhysicalDevice.h>

#include <util/Logger.h>
#include <vulkan/vulkan_to_string.hpp>

namespace gfx
{
	namespace vulkan
	{
		PhysicalDevice::PhysicalDevice(Instance& instance, vk::PhysicalDevice physical_device) :
			instance{ instance },
			VulkanResource{ physical_device, instance.GetAllocationCallbacks() }
		{
			features = physical_device.getFeatures();
			properties = physical_device.getProperties();
			memory_properties = physical_device.getMemoryProperties();

			std::stringstream uuid_ss;
			for (uint8_t byte : properties.pipelineCacheUUID)
				uuid_ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);

			log_info("Found GPU: {}", properties.deviceName.data());
			log_debug("  \tDevice ID: {}", properties.deviceID);
			log_debug("  \tVendor ID: {}", properties.vendorID);
			log_debug("  \tDriver Version: {}", properties.driverVersion);
			log_debug("  \tAPI Version: {}", properties.apiVersion);
			log_debug("  \tDevice Type: {}", vk::to_string(properties.deviceType));
			log_debug("  \tPipeline Cache UUID: {}", uuid_ss.str());

			queue_family_properties = physical_device.getQueueFamilyProperties();

			device_extensions = physical_device.enumerateDeviceExtensionProperties();

			// Display supported extensions
			if (device_extensions.size() > 0)
			{
				log_debug("Device supports the following extensions:");
				for (auto& extension : device_extensions)
				{
					log_debug("  \t{}", extension.extensionName.data());
				}
			}
		}

		DriverVersion PhysicalDevice::GetDriverVersion() const
		{
			DriverVersion version;

			vk::PhysicalDeviceProperties const& properties = GetProperties();
			switch (properties.vendorID)
			{
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

		void* PhysicalDevice::GetExtensionFeatureChain() const
		{
			return last_requested_extension_feature;
		}

		bool PhysicalDevice::IsExtensionSupported(const char* requested_extension) const
		{
			return std::find_if(device_extensions.begin(), device_extensions.end(),
				[requested_extension](auto& device_extension) { return std::strcmp(device_extension.extensionName, requested_extension) == 0; }) != device_extensions.end();
		}

		const vk::PhysicalDeviceFeatures& PhysicalDevice::GetFeatures() const
		{
			return features;
		}

		Instance& PhysicalDevice::GetInstance() const
		{
			return instance;
		}

		const vk::PhysicalDeviceMemoryProperties& PhysicalDevice::GetMemoryProperties() const
		{
			return memory_properties;
		}

		uint32_t PhysicalDevice::GetMemoryType(uint32_t bits, vk::MemoryPropertyFlags properties, vk::Bool32* memory_type_found) const
		{
			for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
			{
				if ((bits & 1) == 1)
				{
					if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
					{
						if (memory_type_found)
						{
							*memory_type_found = true;
						}
						return i;
					}
				}
				bits >>= 1;
			}

			if (memory_type_found)
			{
				*memory_type_found = false;
				return ~0;
			}
			else
			{
				throw std::runtime_error("Could not find a matching memory type");
			}
		}

		const vk::PhysicalDeviceProperties& PhysicalDevice::GetProperties() const
		{
			return properties;
		}

		const std::vector<vk::QueueFamilyProperties>& PhysicalDevice::GetQueueFamilyProperties() const
		{
			return queue_family_properties;
		}

		const vk::PhysicalDeviceFeatures PhysicalDevice::GetRequestedFeatures() const
		{
			return requested_features;
		}

		vk::PhysicalDeviceFeatures& PhysicalDevice::GetMutableRequestedFeatures()
		{
			return requested_features;
		}
	}
}