#include "VkPhysicalDevice.h"

#include <logger.hpp>
#include <volk.h>

#include <functional>

#if defined(ENGINE_DEBUG) || defined(VULKAN_VALIDATION_LAYERS)
#define USE_VALIDATION_LAYERS
#endif

#if defined(USE_VALIDATION_LAYERS) && (defined(VULKAN_VALIDATION_LAYERS_GPU_ASSISTED) || defined(VULKAN_VALIDATION_LAYERS_BEST_PRACTICES) || defined(VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION))
#define USE_VALIDATION_LAYER_FEATURES
#endif

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			namespace
			{
#ifdef USE_VALIDATION_LAYERS
				VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
				{
					// Log debug message
					if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
					{
						log_info("{} - {}: {}", callback_data->messageIdNumber, callback_data->pMessageIdName, callback_data->pMessage);
					}
					else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
					{
						log_info("{} - {}: {}", callback_data->messageIdNumber, callback_data->pMessageIdName, callback_data->pMessage);
					}
					else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
					{
						log_warning("{} - {}: {}", callback_data->messageIdNumber, callback_data->pMessageIdName, callback_data->pMessage);
					}
					else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
					{
						log_error("{} - {}: {}", callback_data->messageIdNumber, callback_data->pMessageIdName, callback_data->pMessage);
					}
					return VK_FALSE;
				}

				static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT /*type*/,
					uint64_t /*object*/, size_t /*location*/, int32_t /*message_code*/,
					const char* layer_prefix, const char* message, void* /*user_data*/)
				{
					if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
					{
						log_error("{}: {}", layer_prefix, message);
					}
					else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
					{
						log_warning("{}: {}", layer_prefix, message);
					}
					else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
					{
						log_warning("{}: {}", layer_prefix, message);
					}
					else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
					{
						log_debug("{}: {}", layer_prefix, message);
					}
					else
					{
						log_info("{}: {}", layer_prefix, message);
					}
					return VK_FALSE;
				}
#endif
			}

			bool ValidateLayers(const std::vector<const char*>& required, const std::vector<vk::LayerProperties>& available)
			{
				for (auto layer : required)
				{
					bool found = false;
					for (auto& available_layer : available)
					{
						if (strcmp(available_layer.layerName, layer) == 0)
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						log_error("Validation Layer {} not found", layer);
						return false;
					}
				}

				return true;
			}

			std::vector<const char*> GetOptimalValidationLayers(const std::vector<vk::LayerProperties>& supported_instance_layers)
			{
				std::vector<std::vector<const char*>> validation_layer_priority_list =
				{
					// The preferred validation layer is "VK_LAYER_KHRONOS_validation"
					{"VK_LAYER_KHRONOS_validation"},

					// Otherwise we fallback to using the LunarG meta layer
					{"VK_LAYER_LUNARG_standard_validation"},

					// Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
					{
						"VK_LAYER_GOOGLE_threading",
						"VK_LAYER_LUNARG_parameter_validation",
						"VK_LAYER_LUNARG_object_tracker",
						"VK_LAYER_LUNARG_core_validation",
						"VK_LAYER_GOOGLE_unique_objects",
					},

					// Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
					{"VK_LAYER_LUNARG_core_validation"}
				};

				for (auto& validation_layers : validation_layer_priority_list)
				{
					if (ValidateLayers(validation_layers, supported_instance_layers))
					{
						return validation_layers;
					}

					log_warning("Couldn't enable validation layers (see log for error) - falling back");
				}

				// Else return nothing
				return {};
			}


			//Result<std::unique_ptr<Instance>, vk::Result> Instance::Construct(const InstanceCreateInfo& create_info)
			//{
			//	std::unique_ptr<Instance> instance = std::make_unique<Instance>(create_info);
			//
			//
			//
			//	return Result<std::unique_ptr<Instance>, vk::Result>::Success(std::move(instance));
			//}


			InstanceBuilder::InstanceBuilder()
			{
			}

			InstanceBuilder& InstanceBuilder::SetApplicationName(std::string_view name)
			{
				application_name = name;
				return *this;
			}

			InstanceBuilder& InstanceBuilder::SetApplicationVersion(uint32_t version)
			{
				application_version = version;
				return *this;
			}

			InstanceBuilder& InstanceBuilder::SetEngineName(std::string_view name)
			{
				engine_name = name;
				return *this;
			}

			InstanceBuilder& InstanceBuilder::SetEngineVersion(uint32_t version)
			{
				engine_version = version;
				return *this;
			}

			InstanceBuilder& InstanceBuilder::SetApiVersion(uint32_t version)
			{
				api_version = version;
				return *this;
			}

			InstanceBuilder& InstanceBuilder::SetRequiredExtensions(std::vector<const char*>&& extensions)
			{
				required_extensions = std::move(extensions);
				return *this;
			}

			InstanceBuilder& InstanceBuilder::AddRequiredExtensions(const std::vector<const char*>& extensions)
			{
				required_extensions.insert(required_extensions.end(), extensions.begin(), extensions.end());
				return *this;
			}

			InstanceBuilder& InstanceBuilder::AddRequiredExtensions(std::span<const char*> extensions)
			{
				required_extensions.insert(required_extensions.end(), extensions.begin(), extensions.end());
				return *this;
			}

			InstanceBuilder& InstanceBuilder::AddRequiredExtension(const char* extension_name)
			{
				required_extensions.emplace_back(extension_name);
				return *this;
			}

			InstanceBuilder& InstanceBuilder::SetOptionalExtensions(std::vector<const char*>&& extensions)
			{
				optional_extensions = std::move(extensions);
				return *this;
			}

			InstanceBuilder& InstanceBuilder::AddOptionalExtensions(const std::vector<const char*>& extensions)
			{
				optional_extensions.insert(optional_extensions.end(), extensions.begin(), extensions.end());
				return *this;
			}

			InstanceBuilder& InstanceBuilder::AddOptionalExtension(const char* extension_name)
			{
				optional_extensions.emplace_back(extension_name);
				return *this;
			}


			InstanceBuilder& InstanceBuilder::SetValidationLayers(std::vector<const char*>&& layers)
			{
				validation_layers = std::move(layers);
				return *this;
			}

			InstanceBuilder& InstanceBuilder::AddValidationLayers(const std::vector<const char*>& layers)
			{
				validation_layers.insert(validation_layers.end(), layers.begin(), layers.end());
				return *this;
			}

			InstanceBuilder& InstanceBuilder::AddValidationLayer(const char* layer_name)
			{
				validation_layers.emplace_back(layer_name);
				return *this;
			}


			InstanceBuilder& InstanceBuilder::SetLayerSettings(std::vector<vk::LayerSettingEXT>&& settings)
			{
				layer_settings = std::move(settings);
				return *this;
			}

			InstanceBuilder& InstanceBuilder::AddLayerSettings(const std::vector<vk::LayerSettingEXT>& settings)
			{
				layer_settings.insert(layer_settings.end(), settings.begin(), settings.end());
				return *this;
			}

			InstanceBuilder& InstanceBuilder::AddLayerSetting(vk::LayerSettingEXT setting)
			{
				layer_settings.emplace_back(setting);
				return *this;
			}


			InstanceBuilder& InstanceBuilder::SetHeadless(bool headless)
			{
				this->headless = headless;
				return *this;
			}

			InstanceBuilder& InstanceBuilder::SetAllocationCallbacks(vk::AllocationCallbacks* allocation_callbacks)
			{
				this->allocation_callbacks = allocation_callbacks;
				return *this;
			}


			Instance InstanceBuilder::Build()
			{
				return Instance(*this);
			}

			std::unique_ptr<Instance> InstanceBuilder::BuildUnique()
			{
				return std::make_unique<Instance>(*this);
			}




			Instance::Instance(const InstanceBuilder& builder) :
				VulkanResource{ nullptr, builder.allocation_callbacks },
				api_version{ builder.api_version },
				selected_gpu_index{ -1 }
			{
				available_extensions = vk::enumerateInstanceExtensionProperties();

#ifdef USE_VALIDATION_LAYERS
				// Check if VK_EXT_debug_utils is supported, which supersedes VK_EXT_Debug_Report
				const bool has_debug_utils = EnableExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				bool has_debug_report = false;

				if (!has_debug_utils)
				{
					has_debug_report = EnableExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
					if (!has_debug_report)
					{
						log_warning("Neither of {} or {} are available; disabling debug reporting", VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
					}
				}
#endif

#if (defined(VULKAN_ENABLE_PORTABILITY))
				EnableExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, available_instance_extensions, enabled_extensions);
				bool portability_enumeration_available = EnableExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, available_instance_extensions, enabled_extensions);
#endif

#ifdef USE_VALIDATION_LAYER_FEATURES
				bool validation_features = false;
				{
					std::vector<vk::ExtensionProperties> available_layer_instance_extensions = vk::enumerateInstanceExtensionProperties(std::string("VK_LAYER_KHRONOS_validation"));
					for (auto& available_extension : available_layer_instance_extensions)
					{
						if (strcmp(available_extension.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0)
						{
							validation_features = true;
							log_info("{} is available, enabling it", VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
							EnableExtension(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
						}
					}
				}
#endif

				// Try to enable headless surface extension if it exists
				if (builder.headless)
				{
					const bool has_headless_surface = EnableExtension(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
					if (!has_headless_surface)
					{
						log_warning("{} is not available, disabling swapchain creation", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
					}
				}
				else
				{
					EnableExtension(VK_KHR_SURFACE_EXTENSION_NAME);
				}

				// VK_KHR_get_physical_device_properties2 is a prerequisite of VK_KHR_performance_query
				// which will be used for stats gathering where available.
				EnableExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

				auto extension_error = false;
				for (auto extension_name : builder.required_extensions)
				{
					if (!EnableExtension(extension_name))
					{
						log_error("Required instance extension {} not available, cannot run", extension_name);
						extension_error = true;
					}
				}

				for (auto extension_name : builder.optional_extensions)
				{
					if (!EnableExtension(extension_name))
					{
						log_error("Optional instance extension {} not available", extension_name);
					}
				}

				if (extension_error)
				{
					throw std::runtime_error("Required instance extensions are missing.");
				}

				std::vector<vk::LayerProperties> supported_validation_layers = vk::enumerateInstanceLayerProperties();

				std::vector<const char*> requested_validation_layers{ builder.validation_layers };

#ifdef USE_VALIDATION_LAYERS
				// Determine the optimal validation layers to enable that are necessary for useful debugging
				std::vector<const char*> optimal_validation_layers = GetOptimalValidationLayers(supported_validation_layers);
				requested_validation_layers.insert(requested_validation_layers.end(), optimal_validation_layers.begin(), optimal_validation_layers.end());

#if defined(VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION)
				requested_validation_layers.emplace_back("VK_LAYER_KHRONOS_synchronization2");
#endif // VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION
#endif // USE_VALIDATION_LAYERS

				if (ValidateLayers(requested_validation_layers, supported_validation_layers))
				{
					log_info("Enabled Validation Layers:");
					for (const auto& layer : requested_validation_layers)
					{
						log_info("	\t{}", layer);
					}
				}
				else
				{
					throw std::runtime_error("Required validation layers are missing.");
				}

				vk::ApplicationInfo app_info(builder.application_name.c_str(), builder.application_version, builder.engine_name.c_str(), builder.engine_version, builder.api_version);

				vk::InstanceCreateInfo instance_info({}, &app_info, requested_validation_layers, enabled_extensions);

#ifdef USE_VALIDATION_LAYERS
				vk::DebugUtilsMessengerCreateInfoEXT debug_utils_create_info;
				vk::DebugReportCallbackCreateInfoEXT debug_report_create_info;
				if (has_debug_utils)
				{
					debug_utils_create_info =
						vk::DebugUtilsMessengerCreateInfoEXT({},
							vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
							vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
							DebugUtilsMessengerCallback);

					instance_info.pNext = &debug_utils_create_info;
				}
				else if (has_debug_report)
				{
					debug_report_create_info = vk::DebugReportCallbackCreateInfoEXT(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning, DebugCallback);

					instance_info.pNext = &debug_report_create_info;
				}
#endif

#if (defined(VULKAN_ENABLE_PORTABILITY))
				if (portability_enumeration_available)
				{
					instance_info.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
				}
#endif

#ifdef USE_VALIDATION_LAYER_FEATURES
				vk::ValidationFeaturesEXT validation_features_info;
				std::vector<vk::ValidationFeatureEnableEXT> enable_features{};
				if (validation_features)
				{
#if defined(VULKAN_VALIDATION_LAYERS_GPU_ASSISTED)
					enable_features.emplace_back(vk::ValidationFeatureEnableEXT::eGpuAssistedReserveBindingSlot);
					enable_features.emplace_back(vk::ValidationFeatureEnableEXT::eGpuAssisted);
#endif
#if defined(VULKAN_VALIDATION_LAYERS_BEST_PRACTICES)
					enable_features.emplace_back(vk::ValidationFeatureEnableEXT::eBestPractices);
#endif
#if defined(VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION)
					enable_features.emplace_back(vk::ValidationFeatureEnableEXT::eSynchronizationValidation);
#endif
					validation_features_info.setEnabledValidationFeatures(enable_features);
					validation_features_info.pNext = instance_info.pNext;
					instance_info.pNext = &validation_features_info;
				}
#endif

				vk::LayerSettingsCreateInfoEXT layerSettingsCreateInfo;

				// If layer settings extension enabled by sample, then activate layer settings during instance creation
				if (IsEnabled(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME))
				{
					layerSettingsCreateInfo.settingCount = static_cast<uint32_t>(builder.layer_settings.size());
					layerSettingsCreateInfo.pSettings = builder.layer_settings.data();
					layerSettingsCreateInfo.pNext = instance_info.pNext;
					instance_info.pNext = &layerSettingsCreateInfo;
				}

				// Create the Vulkan instance
				handle = vk::createInstance(instance_info);

				// initialize the Vulkan-Hpp default dispatcher on the instance
				VULKAN_HPP_DEFAULT_DISPATCHER.init(handle);

				// Need to load volk for all the not-yet Vulkan-Hpp calls
				volkLoadInstance(handle);

#ifdef USE_VALIDATION_LAYERS
				if (has_debug_utils)
				{
					debug_utils_messenger = handle.createDebugUtilsMessengerEXT(debug_utils_create_info);
				}
				else if (has_debug_report)
				{
					debug_report_callback = handle.createDebugReportCallbackEXT(debug_report_create_info);
				}
#endif

				// Querying valid physical devices on the machine
				std::vector<vk::PhysicalDevice> physical_devices = handle.enumeratePhysicalDevices();
				if (physical_devices.empty())
				{
					throw std::runtime_error("Couldn't find a physical device that supports Vulkan.");
				}

				// Create gpus wrapper objects from the vk::PhysicalDevice's
				for (auto& physical_device : physical_devices)
				{
					gpus.emplace_back(std::make_unique<PhysicalDevice>(*this, physical_device));
				}
			}

			Instance::Instance(vk::Instance instance) :
				VulkanResource{ instance, nullptr }
			{
				if (handle)
				{
					// Querying valid physical devices on the machine
					std::vector<vk::PhysicalDevice> physical_devices = handle.enumeratePhysicalDevices();
					if (physical_devices.empty())
					{
						throw std::runtime_error("Couldn't find a physical device that supports Vulkan.");
					}

					// Create gpus wrapper objects from the vk::PhysicalDevice's
					for (auto& physical_device : physical_devices)
					{
						gpus.emplace_back(std::make_unique<PhysicalDevice>(*this, physical_device));
					}
				}
				else
				{
					throw std::runtime_error("Instance not valid");
				}
			}

			Instance::~Instance()
			{
#ifdef USE_VALIDATION_LAYERS
				if (debug_utils_messenger)
				{
					handle.destroyDebugUtilsMessengerEXT(debug_utils_messenger);
				}
				if (debug_report_callback)
				{
					handle.destroyDebugReportCallbackEXT(debug_report_callback);
				}
#endif

				if (handle)
				{
					handle.destroy();
				}
			}

			const std::vector<const char*>& Instance::GetExtensions()
			{
				return enabled_extensions;
			}

			uint32_t Instance::GetApiVersion() const
			{
				return api_version;
			}

			const std::vector<std::unique_ptr<PhysicalDevice>>& Instance::GetAllFoundGpu() const
			{
				return gpus;
			}

			PhysicalDevice& Instance::GetBestDevice() const
			{
				if (selected_gpu_index < 0)
				{
					throw std::runtime_error("Device is not selected yet.");
				}

				return *gpus[selected_gpu_index];
			}

			void Instance::SelectBestDevice(vk::SurfaceKHR& surface, const std::vector<const char*>& requested_extensions)
			{
				assert(!gpus.empty() && "No physical devices were found on the system.");

				if (selected_gpu_index >= 0)
				{
					log_info("Explicitly selecting GPU {}", selected_gpu_index);
					if (selected_gpu_index > gpus.size() - 1)
					{
						throw std::runtime_error("Selected GPU index is not within no. of available GPUs");
					}
					return;
				}

				int32_t fallback_device_index{ -1 };
				for (int32_t device_index = 0; device_index < gpus.size(); ++device_index)
				{
					auto& device = gpus[device_index];
					auto& properties = device->GetProperties();

					if (properties.apiVersion < api_version)
					{
						log_warning("Device is not supported. Required API version: {}, but device supporting {}.", api_version, properties.apiVersion);
						continue;
					}

					// Device should support all required extensions
					for (const auto& extension_name : requested_extensions)
					{
						if (!device->IsExtensionSupported(extension_name))
						{
							log_info("Device {} is not supporting required extension: {}.", properties.deviceName.data(), extension_name);
						}
					}

					// Discrete gpu is in higher priority
					if (properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
					{
						fallback_device_index = device_index;
						continue;
					}

					// Check that any device queue family is support present
					if (surface)
					{
						bool support_present{ false };

						auto family_properties = device->GetQueueFamilyProperties();
						for (int32_t index = 0; index < family_properties.size(); ++index)
						{
							if (Handle(*device).getSurfaceSupportKHR(index, surface))
							{
								support_present = true;
								break;
							}
						}

						if (!support_present)
							continue;
					}

					selected_gpu_index = device_index;
				}

				if (selected_gpu_index < 0)
				{
					if (fallback_device_index < 0)
					{
						log_error("Couldn't find a physical device.");
						throw std::runtime_error("Couldn't find a physical device.");
					}

					selected_gpu_index = fallback_device_index;
				}
			}

			bool Instance::IsEnabled(const char* extension) const
			{
				return std::find_if(enabled_extensions.begin(), enabled_extensions.end(), [extension](const char* enabled_extension) { return strcmp(extension, enabled_extension) == 0; }) != enabled_extensions.end();
			}

			bool Instance::EnableExtension(const char* extension_name)
			{
				for (auto& avail_ext_it : available_extensions)
				{
					if (strcmp(avail_ext_it.extensionName, extension_name) == 0)
					{
						auto it = std::find_if(enabled_extensions.begin(), enabled_extensions.end(),
							[extension_name](const char* enabled_ext_name) {
								return strcmp(enabled_ext_name, extension_name) == 0;
							});
						if (it != enabled_extensions.end())
						{
							// Extension is already enabled
						}
						else
						{
							log_info("Extension {} found, enabling it", extension_name);
							enabled_extensions.emplace_back(extension_name);
						}
						return true;
					}
				}

				log_info("Extension {} not found", extension_name);
				return false;
			}
		}
	}
}