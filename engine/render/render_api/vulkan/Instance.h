#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <engine/render/render_api/vulkan/VkResource.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class PhysicalDevice;

			struct InstanceCreateInfo
			{
				std::string application_name;
				std::vector<const char*>& required_extensions;
				std::vector<const char*>& required_validation_layers;
				std::vector<vk::LayerSettingEXT>& required_layer_settings;
				bool headless = false;
				uint32_t api_version = VK_API_VERSION_1_0;
			};

			/**
			 * @brief Returns a list of Khronos/LunarG supported validation layers
			 *        Attempting to enable them in order of preference, starting with later Vulkan SDK versions
			 * @param supported_instance_layers A list of validation layers to check against
			 */
			std::vector<const char*> GetOptimalValidationLayers(const std::vector<vk::LayerProperties>& supported_instance_layers);

			/**
	 * @brief A wrapper class for vk::Instance
	 *
	 * This class is responsible for initializing the dispatcher, enumerating over all available extensions and validation layers
	 * enabling them if they exist, setting up debug messaging and querying all the physical devices existing on the machine.
	 */
			class Instance final : public VulkanResource<vk::Instance>
			{
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
				Instance(const InstanceCreateInfo& create_info);

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

				const std::vector<const char*>& GetExtensions();

				/**
				 * @brief Just returns vulkan api version that was used on init
				 * @returns Vulkan api version
				 */
				uint32_t GetApiVersion() const;

				/**
				 * @brief Tries to find the first available discrete GPU
				 * @returns All physical devices found on 
				 */
				const std::vector<std::unique_ptr<PhysicalDevice>>& GetAllFoundGpu() const;

				/**
				 * @brief Returns physical device that was selected in method SelectBestDevice
				 * @returns Best physical device
				 */
				PhysicalDevice& GetBestDevice() const;

				/**
				 * @brief Tries to find the first available discrete GPU that can render to the given surface and support
				 * @param surface to test against
				 * @param required device extensions
				 */
				void SelectBestDevice(vk::SurfaceKHR& surface, const std::vector<const char*>& requested_extensions);

				/**
				 * @brief Checks if the given extension is enabled in the vk::Instance
				 * @param extension An extension to check
				 */
				bool IsEnabled(const char* extension) const;

			private:
				/**
				 * @brief Queries the instance for the physical devices on the machine
				 */
				void QueryGpus();

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
	}
}