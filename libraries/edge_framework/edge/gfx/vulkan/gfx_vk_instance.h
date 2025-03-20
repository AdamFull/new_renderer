#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "../../util/Result.hpp"
#include "gfx_vk_resource.h"

namespace edge::gfx::vulkan
{
	class PhysicalDevice;

	class Instance;

	class InstanceBuilder {
	public:
		std::string application_name{ "app" };
		uint32_t application_version{ 0u };
		std::string engine_name{ "engine" };
		uint32_t engine_version{ 0u };
		uint32_t api_version{ VK_API_VERSION_1_1 };

		std::vector<const char*> required_extensions;
		std::vector<const char*> optional_extensions;

		std::vector<const char*> validation_layers;

		std::vector<vk::LayerSettingEXT> layer_settings;

		vk::AllocationCallbacks* allocation_callbacks{ nullptr };

		bool headless;

		InstanceBuilder();

		InstanceBuilder& set_application_name(std::string_view name);
		InstanceBuilder& set_application_version(uint32_t version);

		InstanceBuilder& set_engine_name(std::string_view name);
		InstanceBuilder& set_engine_version(uint32_t version);

		InstanceBuilder& set_api_version(uint32_t version);

		InstanceBuilder& set_required_extensions(std::vector<const char*>&& extensions);
		InstanceBuilder& add_required_extensions(const std::vector<const char*>& extensions);
		InstanceBuilder& add_required_extensions(std::span<const char*> extensions);
		InstanceBuilder& add_required_extension(const char* extension_name);

		InstanceBuilder& set_optional_extensions(std::vector<const char*>&& extensions);
		InstanceBuilder& add_optional_extensions(const std::vector<const char*>& extensions);
		InstanceBuilder& add_optional_extension(const char* extension_name);

		InstanceBuilder& set_validation_layers(std::vector<const char*>&& layers);
		InstanceBuilder& add_validationLayers(const std::vector<const char*>& layers);
		InstanceBuilder& add_validation_layer(const char* layer_name);

		InstanceBuilder& set_layer_settings(std::vector<vk::LayerSettingEXT>&& layer_settings);
		InstanceBuilder& add_layer_settings(const std::vector<vk::LayerSettingEXT>& layer_settings);
		InstanceBuilder& add_layer_setting(vk::LayerSettingEXT layer_setting);

		InstanceBuilder& set_headless(bool headless);
		InstanceBuilder& set_allocation_callbacks(vk::AllocationCallbacks* allocation_callbacks);

		Instance build();
		std::unique_ptr<Instance> build_unique();
	};

	/**
	 * @brief Returns a list of Khronos/LunarG supported validation layers
	 *        Attempting to enable them in order of preference, starting with later Vulkan SDK versions
	 * @param supported_instance_layers A list of validation layers to check against
	 */
	std::vector<const char*> get_optimal_validation_layers(const std::vector<vk::LayerProperties>& supported_instance_layers);

	/**
	 * @brief A wrapper class for vk::Instance
	 *
	 * This class is responsible for initializing the dispatcher, enumerating over all available extensions and validation layers
	 * enabling them if they exist, setting up debug messaging and querying all the physical devices existing on the machine.
	 */
	class Instance final : public VulkanResource<vk::Instance> {
	public:
		/**
		 * @brief Initializes the connection to Vulkan
		 * @param application_name The name of the application
		 * @param required_extensions The extensions requested to be enabled
		 * @param required_validation_layers The validation layers to be enabled
		 * @param required_layer_settings The layer settings to be enabled
		 * @param headless Whether the application is requesting a headless setup or not
		 * @param api_version The Vulkan API version that the instance will be using
		 * @throws runtime_error if the required extensions and validation layers are not found
		 */
		Instance(const InstanceBuilder& builder);

		/**
		 * @brief Queries the GPUs of a vk::Instance that is already created
		 * @param instance A valid vk::Instance
		 */
		Instance(vk::Instance instance);

		Instance(const Instance&) = delete;

		Instance(Instance&&) = delete;

		~Instance();

		Instance& operator=(const Instance&) = delete;

		Instance& operator=(Instance&&) = delete;

		auto get_extensions() const noexcept -> std::vector<const char*> const& {
			return enabled_extensions;
		}

		/**
		 * @brief Just returns vulkan api version that was used on init
		 * @returns Vulkan api version
		 */
		auto get_api_version() const noexcept -> uint32_t {
			return api_version;
		}

		/**
		 * @brief Tries to find the first available discrete GPU
		 * @returns All physical devices found on
		 */
		auto get_all_found_gpu() const noexcept -> std::vector<std::unique_ptr<PhysicalDevice>> const&
		{
			return gpus;
		}

		/**
		 * @brief Returns physical device that was selected in method select_best_device
		 * @returns Best physical device
		 */
		auto get_best_device() const->PhysicalDevice&;

		/**
		 * @brief Tries to find the first available discrete GPU that can render to the given surface and support
		 * @param surface to test against
		 * @param required device extensions
		 */
		void select_best_device(vk::SurfaceKHR& surface, const std::vector<const char*>& requested_extensions);

		/**
		 * @brief Checks if the given extension is enabled in the vk::Instance
		 * @param extension An extension to check
		 */
		auto is_enabled(const char* extension) const noexcept -> bool;

	private:
		/**
		* @brief Enables extension when it supported
		* @param Requested extension name
		* @returns True if extension is supported or enabled
		*/
		bool enable_extension(const char* extension_name);

	private:
		/**
		 * @brief Vulkan api version
		 */
		uint32_t api_version;

		/**
		 * @brief Best gpu index
		 */
		int32_t selected_gpu_index;

		/**
		 * @brief All avaliable extensions
		 */
		std::vector<vk::ExtensionProperties> available_extensions;

		/**
		 * @brief The enabled extensions
		 */
		std::vector<const char*> enabled_extensions;

#if defined(ENGINE_DEBUG) || defined(VULKAN_VALIDATION_LAYERS)
		/**
		 * @brief Debug utils messenger callback for VK_EXT_Debug_Utils
		 */
		vk::DebugUtilsMessengerEXT debug_utils_messenger;

		/**
		 * @brief The debug report callback
		 */
		vk::DebugReportCallbackEXT debug_report_callback;
#endif

		/**
		 * @brief The physical devices found on the machine
		 */
		std::vector<std::unique_ptr<PhysicalDevice>> gpus;
	};
}