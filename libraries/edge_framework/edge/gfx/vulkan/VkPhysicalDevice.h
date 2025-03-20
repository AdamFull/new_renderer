#pragma once

#include <map>

#include "VkInstance.h"

namespace edge::gfx::vulkan
{
	struct DriverVersion {
		uint16_t major;
		uint16_t minor;
		uint16_t patch;
	};

	/**
	 * @brief A wrapper class for vk::PhysicalDevice
	 *
	 * This class is responsible for handling gpu features, properties, and queue families for the device creation.
	 */
	class PhysicalDevice : public VulkanResource<vk::PhysicalDevice> {
	public:
		PhysicalDevice(Instance& instance, vk::PhysicalDevice physical_device);

		PhysicalDevice(const PhysicalDevice&) = delete;
		PhysicalDevice(PhysicalDevice&&) = delete;
		PhysicalDevice& operator=(const PhysicalDevice&) = delete;
		PhysicalDevice& operator=(PhysicalDevice&&) = delete;

		/**
		 * @return The version of the driver
		 */
		auto get_driver_version() const noexcept -> DriverVersion;

		/**
		 * @brief Used at logical device creation to pass the extensions feature chain to vkCreateDevice
		 * @returns A void pointer to the start of the extension linked list
		 */
		auto get_extension_feature_chain() const noexcept -> void* {
			return last_requested_extension_feature;
		}

		auto is_extension_supported(const char* requested_extension) const noexcept -> bool;

		auto get_features() const noexcept -> vk::PhysicalDeviceFeatures const& {
			return features;
		}

		auto get_instance() const noexcept -> Instance& {
			return instance;
		}

		auto get_memory_properties() const noexcept -> vk::PhysicalDeviceMemoryProperties const& {
			return memory_properties;
		}

		/**
		 * @brief Checks that a given memory type is supported by the GPU
		 * @param bits The memory requirement type bits
		 * @param properties The memory property to search for
		 * @param memory_type_found True if found, false if not found
		 * @returns The memory type index of the found memory type
		 */
		auto get_memory_type(uint32_t bits, vk::MemoryPropertyFlags properties, vk::Bool32* memory_type_found = nullptr) const noexcept -> uint32_t;

		const vk::PhysicalDeviceProperties& get_properties() const {
			return properties;
		}

		auto get_queue_family_properties() const noexcept -> std::vector<vk::QueueFamilyProperties> const& {
			return queue_family_properties;
		}

		auto get_requested_features() const noexcept -> const vk::PhysicalDeviceFeatures& {
			return requested_features;
		}

		auto get_mutable_requested_features() noexcept -> vk::PhysicalDeviceFeatures& {
			return requested_features;
		}

		/**
		 * @brief Requests a third party extension to be used by the framework
		 *        If the feature extension requires you to ask for certain features to be enabled, you can
		 *        modify the struct returned by this function, it will propagate the changes to the logical
		 *        device.
		 * @returns The extension feature struct
		 */
		template <typename StructureType>
		StructureType& request_extension_features() {
			// We cannot request extension features if the physical device properties 2 instance extension isn't enabled
			if (!instance.is_enabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
				throw std::runtime_error("Couldn't request feature from device as " + std::string(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) + " isn't enabled!");
			}

			// If the type already exists in the map, return a casted pointer to get the extension feature struct
			vk::StructureType structureType = StructureType::structureType;        // need to instantiate this value to be usable in find()!
			auto extension_features_it = extension_features.find(structureType);
			if (extension_features_it != extension_features.end()) {
				return *static_cast<StructureType*>(extension_features_it->second.get());
			}

			// Get the extension feature
			vk::StructureChain<vk::PhysicalDeviceFeatures2KHR, StructureType> featureChain = handle_.getFeatures2KHR<vk::PhysicalDeviceFeatures2KHR, StructureType>();

			// Insert the extension feature into the extension feature map so its ownership is held
			extension_features.insert({ structureType, std::make_shared<StructureType>(featureChain.template get<StructureType>()) });

			// Pull out the dereferenced void pointer, we can assume its type based on the template
			auto* extension_ptr = static_cast<StructureType*>(extension_features.find(structureType)->second.get());

			// If an extension feature has already been requested, we shift the linked list down by one
			// Making this current extension the new base pointer
			if (last_requested_extension_feature) {
				extension_ptr->pNext = last_requested_extension_feature;
			}
			last_requested_extension_feature = extension_ptr;

			return *extension_ptr;
		}

		/**
		 * @brief Sets whether or not the first graphics queue should have higher priority than other queues.
		 * Very specific feature which is used by async compute samples.
		 * @param enable If true, present queue will have prio 1.0 and other queues have prio 0.5.
		 * Default state is false, where all queues have 0.5 priority.
		 */
		void set_high_priority_graphics_queue_enable(bool enable) {
			high_priority_graphics_queue = enable;
		}

		/**
		 * @brief Returns high priority graphics queue state.
		 * @return High priority state.
		 */
		bool has_high_priority_graphics_queue() const {
			return high_priority_graphics_queue;
		}

	private:
		// Handle to the Vulkan instance
		Instance& instance;

		// The features that this GPU supports
		vk::PhysicalDeviceFeatures features;

		// The extensions that this GPU supports
		std::vector<vk::ExtensionProperties> device_extensions;

		// The GPU properties
		vk::PhysicalDeviceProperties properties;

		// The GPU memory properties
		vk::PhysicalDeviceMemoryProperties memory_properties;

		// The GPU queue family properties
		std::vector<vk::QueueFamilyProperties> queue_family_properties;

		// The features that will be requested to be enabled in the logical device
		vk::PhysicalDeviceFeatures requested_features;

		// The extension feature pointer
		void* last_requested_extension_feature{ nullptr };

		// Holds the extension feature structures, we use a map to retain an order of requested structures
		std::map<vk::StructureType, std::shared_ptr<void>> extension_features;

		bool high_priority_graphics_queue{ false };
	};
}