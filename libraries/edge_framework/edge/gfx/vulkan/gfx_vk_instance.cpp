#include "gfx_vk_physical_device.h"

#include "../../util/Logger.h"
#include <volk.h>

#include <functional>

#if defined(ENGINE_DEBUG) || defined(VULKAN_VALIDATION_LAYERS)
#define USE_VALIDATION_LAYERS
#endif

#if defined(USE_VALIDATION_LAYERS) && (defined(VULKAN_VALIDATION_LAYERS_GPU_ASSISTED) || defined(VULKAN_VALIDATION_LAYERS_BEST_PRACTICES) || defined(VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION))
#define USE_VALIDATION_LAYER_FEATURES
#endif

namespace edge::gfx::vulkan
{
	namespace
	{
#ifdef USE_VALIDATION_LAYERS
		VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
			// Log debug message
			if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
				EDGE_LOGI("{} - {}: {}", callback_data->messageIdNumber, callback_data->pMessageIdName, callback_data->pMessage);
			}
			else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
				EDGE_LOGI("{} - {}: {}", callback_data->messageIdNumber, callback_data->pMessageIdName, callback_data->pMessage);
			}
			else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
				EDGE_LOGW("{} - {}: {}", callback_data->messageIdNumber, callback_data->pMessageIdName, callback_data->pMessage);
			}
			else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
				EDGE_LOGE("{} - {}: {}", callback_data->messageIdNumber, callback_data->pMessageIdName, callback_data->pMessage);
			}
			return VK_FALSE;
		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT /*type*/,
			uint64_t /*object*/, size_t /*location*/, int32_t /*message_code*/,
			const char* layer_prefix, const char* message, void* /*user_data*/) {
			if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
				EDGE_LOGE("{}: {}", layer_prefix, message);
			}
			else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
				EDGE_LOGW("{}: {}", layer_prefix, message);
			}
			else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
				EDGE_LOGW("{}: {}", layer_prefix, message);
			}
			else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
				EDGE_LOGD("{}: {}", layer_prefix, message);
			}
			else {
				EDGE_LOGI("{}: {}", layer_prefix, message);
			}
			return VK_FALSE;
		}
#endif
	}

	bool validate_layers(const std::vector<const char*>& required, const std::vector<vk::LayerProperties>& available) {
		for (auto layer : required) {
			bool found = false;
			for (auto& available_layer : available) {
				if (strcmp(available_layer.layerName, layer) == 0) {
					found = true;
					break;
				}
			}

			if (!found) {
				EDGE_LOGE("Validation Layer {} not found", layer);
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> get_optimal_validation_layers(const std::vector<vk::LayerProperties>& supported_instance_layers) {
		std::vector<std::vector<const char*>> validation_layer_priority_list = {
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

		for (auto& validation_layers : validation_layer_priority_list) {
			if (validate_layers(validation_layers, supported_instance_layers)) {
				return validation_layers;
			}

			EDGE_LOGW("Couldn't enable validation layers (see log for error) - falling back");
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


	InstanceBuilder::InstanceBuilder() {
	}

	InstanceBuilder& InstanceBuilder::set_application_name(std::string_view name) {
		application_name = name;
		return *this;
	}

	InstanceBuilder& InstanceBuilder::set_application_version(uint32_t version) {
		application_version = version;
		return *this;
	}

	InstanceBuilder& InstanceBuilder::set_engine_name(std::string_view name) {
		engine_name = name;
		return *this;
	}

	InstanceBuilder& InstanceBuilder::set_engine_version(uint32_t version) {
		engine_version = version;
		return *this;
	}

	InstanceBuilder& InstanceBuilder::set_api_version(uint32_t version) {
		api_version = version;
		return *this;
	}

	InstanceBuilder& InstanceBuilder::set_required_extensions(std::vector<const char*>&& extensions) {
		required_extensions = std::move(extensions);
		return *this;
	}

	InstanceBuilder& InstanceBuilder::add_required_extensions(const std::vector<const char*>& extensions) {
		required_extensions.insert(required_extensions.end(), extensions.begin(), extensions.end());
		return *this;
	}

	InstanceBuilder& InstanceBuilder::add_required_extensions(std::span<const char*> extensions) {
		required_extensions.insert(required_extensions.end(), extensions.begin(), extensions.end());
		return *this;
	}

	InstanceBuilder& InstanceBuilder::add_required_extension(const char* extension_name) {
		required_extensions.emplace_back(extension_name);
		return *this;
	}

	InstanceBuilder& InstanceBuilder::set_optional_extensions(std::vector<const char*>&& extensions) {
		optional_extensions = std::move(extensions);
		return *this;
	}

	InstanceBuilder& InstanceBuilder::add_optional_extensions(const std::vector<const char*>& extensions) {
		optional_extensions.insert(optional_extensions.end(), extensions.begin(), extensions.end());
		return *this;
	}

	InstanceBuilder& InstanceBuilder::add_optional_extension(const char* extension_name) {
		optional_extensions.emplace_back(extension_name);
		return *this;
	}


	InstanceBuilder& InstanceBuilder::set_validation_layers(std::vector<const char*>&& layers) {
		validation_layers = std::move(layers);
		return *this;
	}

	InstanceBuilder& InstanceBuilder::add_validationLayers(const std::vector<const char*>& layers) {
		validation_layers.insert(validation_layers.end(), layers.begin(), layers.end());
		return *this;
	}

	InstanceBuilder& InstanceBuilder::add_validation_layer(const char* layer_name) {
		validation_layers.emplace_back(layer_name);
		return *this;
	}


	InstanceBuilder& InstanceBuilder::set_layer_settings(std::vector<vk::LayerSettingEXT>&& settings) {
		layer_settings = std::move(settings);
		return *this;
	}

	InstanceBuilder& InstanceBuilder::add_layer_settings(const std::vector<vk::LayerSettingEXT>& settings) {
		layer_settings.insert(layer_settings.end(), settings.begin(), settings.end());
		return *this;
	}

	InstanceBuilder& InstanceBuilder::add_layer_setting(vk::LayerSettingEXT setting) {
		layer_settings.emplace_back(setting);
		return *this;
	}


	InstanceBuilder& InstanceBuilder::set_headless(bool headless) {
		this->headless = headless;
		return *this;
	}

	InstanceBuilder& InstanceBuilder::set_allocation_callbacks(vk::AllocationCallbacks* allocation_callbacks) {
		this->allocation_callbacks = allocation_callbacks;
		return *this;
	}


	Instance InstanceBuilder::build() {
		return Instance(*this);
	}

	std::unique_ptr<Instance> InstanceBuilder::build_unique() {
		return std::make_unique<Instance>(*this);
	}




	Instance::Instance(const InstanceBuilder& builder) :
		VulkanResource{ nullptr, builder.allocation_callbacks },
		api_version{ builder.api_version },
		selected_gpu_index{ -1 } {

		available_extensions = vk::enumerateInstanceExtensionProperties();

#ifdef USE_VALIDATION_LAYERS
		// Check if VK_EXT_debug_utils is supported, which supersedes VK_EXT_Debug_Report
		const bool has_debug_utils = enable_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		bool has_debug_report = false;

		if (!has_debug_utils) {
			has_debug_report = enable_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			if (!has_debug_report) {
				EDGE_LOGW("Neither of {} or {} are available; disabling debug reporting", VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			}
		}
#endif

#if (defined(VULKAN_ENABLE_PORTABILITY))
		enable_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, available_instance_extensions, enabled_extensions);
		bool portability_enumeration_available = enable_extension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, available_instance_extensions, enabled_extensions);
#endif

#ifdef USE_VALIDATION_LAYER_FEATURES
		bool validation_features = false;
		{
			std::vector<vk::ExtensionProperties> available_layer_instance_extensions = vk::enumerateInstanceExtensionProperties(std::string("VK_LAYER_KHRONOS_validation"));
			for (auto& available_extension : available_layer_instance_extensions) {
				if (strcmp(available_extension.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0) {
					validation_features = true;
					EDGE_LOGI("[VkInstance] {} is available, enabling it", VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
					enable_extension(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
				}
			}
		}
#endif

		// Try to enable headless surface extension if it exists
		if (builder.headless) {
			const bool has_headless_surface = enable_extension(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
			if (!has_headless_surface) {
				EDGE_LOGW("[VkInstance] {} is not available, disabling swapchain creation", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
			}
		}
		else {
			enable_extension(VK_KHR_SURFACE_EXTENSION_NAME);
		}

		// VK_KHR_get_physical_device_properties2 is a prerequisite of VK_KHR_performance_query
		// which will be used for stats gathering where available.
		enable_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		auto extension_error = false;
		for (auto extension_name : builder.required_extensions) {
			if (!enable_extension(extension_name)) {
				EDGE_LOGE("[VkInstance] Required instance extension {} not available, cannot run", extension_name);
				extension_error = true;
			}
		}

		for (auto extension_name : builder.optional_extensions) {
			if (!enable_extension(extension_name)) {
				EDGE_LOGE("[VkInstance] Optional instance extension {} not available", extension_name);
			}
		}

		if (extension_error) {
			EDGE_LOGE("[VkInstance] Required instance extensions are missing.");
		}

		std::vector<vk::LayerProperties> supported_validation_layers = vk::enumerateInstanceLayerProperties();

		std::vector<const char*> requested_validation_layers{ builder.validation_layers };

#ifdef USE_VALIDATION_LAYERS
		// Determine the optimal validation layers to enable that are necessary for useful debugging
		std::vector<const char*> optimal_validation_layers = get_optimal_validation_layers(supported_validation_layers);
		requested_validation_layers.insert(requested_validation_layers.end(), optimal_validation_layers.begin(), optimal_validation_layers.end());

#if defined(VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION)
		requested_validation_layers.emplace_back("VK_LAYER_KHRONOS_synchronization2");
#endif // VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION
#endif // USE_VALIDATION_LAYERS

		if (validate_layers(requested_validation_layers, supported_validation_layers)) {
			EDGE_LOGI("[VkInstance] Enabled Validation Layers:");
			for (const auto& layer : requested_validation_layers) {
				EDGE_LOGI("	\t{}", layer);
			}
		}
		else {
			EDGE_LOGE("[VkInstance] Required validation layers are missing.");
		}

		vk::ApplicationInfo app_info(builder.application_name.c_str(), builder.application_version, builder.engine_name.c_str(), builder.engine_version, builder.api_version);

		vk::InstanceCreateInfo instance_info({}, &app_info, requested_validation_layers, enabled_extensions);

#ifdef USE_VALIDATION_LAYERS
		vk::DebugUtilsMessengerCreateInfoEXT debug_utils_create_info;
		vk::DebugReportCallbackCreateInfoEXT debug_report_create_info;
		if (has_debug_utils) {
			debug_utils_create_info =
				vk::DebugUtilsMessengerCreateInfoEXT({},
					vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
					vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
					debug_utils_messenger_callback);

			instance_info.pNext = &debug_utils_create_info;
		}
		else if (has_debug_report)
		{
			debug_report_create_info = vk::DebugReportCallbackCreateInfoEXT(
				vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning, 
				debug_callback);
			instance_info.pNext = &debug_report_create_info;
		}
#endif

#if (defined(VULKAN_ENABLE_PORTABILITY))
		if (portability_enumeration_available) {
			instance_info.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
		}
#endif

#ifdef USE_VALIDATION_LAYER_FEATURES
		vk::ValidationFeaturesEXT validation_features_info;
		std::vector<vk::ValidationFeatureEnableEXT> enable_features{};
		if (validation_features) {
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
		if (is_enabled(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME)) {
			layerSettingsCreateInfo.settingCount = static_cast<uint32_t>(builder.layer_settings.size());
			layerSettingsCreateInfo.pSettings = builder.layer_settings.data();
			layerSettingsCreateInfo.pNext = instance_info.pNext;
			instance_info.pNext = &layerSettingsCreateInfo;
		}

		// Create the Vulkan instance
		handle_ = vk::createInstance(instance_info);

		// initialize the Vulkan-Hpp default dispatcher on the instance
		VULKAN_HPP_DEFAULT_DISPATCHER.init(handle_);

		// Need to load volk for all the not-yet Vulkan-Hpp calls
		volkLoadInstance(handle_);

#ifdef USE_VALIDATION_LAYERS
		if (has_debug_utils) {
			debug_utils_messenger = handle_.createDebugUtilsMessengerEXT(debug_utils_create_info);
		}
		else if (has_debug_report) {
			debug_report_callback = handle_.createDebugReportCallbackEXT(debug_report_create_info);
		}
#endif

		// Querying valid physical devices on the machine
		std::vector<vk::PhysicalDevice> physical_devices = handle_.enumeratePhysicalDevices();
		if (physical_devices.empty()) {
			EDGE_LOGE("[VkInstance] Couldn't find a physical device that supports Vulkan.");
		}

		// Create gpus wrapper objects from the vk::PhysicalDevice's
		for (auto& physical_device : physical_devices) {
			gpus.emplace_back(std::make_unique<PhysicalDevice>(*this, physical_device));
		}
	}

	Instance::Instance(vk::Instance instance) :
		VulkanResource{ instance, nullptr } {
		if (handle_) {
			// Querying valid physical devices on the machine
			std::vector<vk::PhysicalDevice> physical_devices = handle_.enumeratePhysicalDevices();
			if (physical_devices.empty()) {
				EDGE_LOGE("[VkInstance] Couldn't find a physical device that supports Vulkan.");
			}

			// Create gpus wrapper objects from the vk::PhysicalDevice's
			for (auto& physical_device : physical_devices) {
				gpus.emplace_back(std::make_unique<PhysicalDevice>(*this, physical_device));
			}
		}
		else {
			EDGE_LOGE("[VkInstance] Instance not valid");
		}
	}

	Instance::~Instance() {
#ifdef USE_VALIDATION_LAYERS
		if (debug_utils_messenger) {
			handle_.destroyDebugUtilsMessengerEXT(debug_utils_messenger);
		}
		if (debug_report_callback) {
			handle_.destroyDebugReportCallbackEXT(debug_report_callback);
		}
#endif

		if (handle_) {
			handle_.destroy();
		}
	}

	auto Instance::get_best_device() const -> PhysicalDevice& {
		if (selected_gpu_index < 0) {
			EDGE_LOGE("[VkInstance] Device is not selected yet.");
		}

		return *gpus[selected_gpu_index];
	}

	void Instance::select_best_device(vk::SurfaceKHR& surface, const std::vector<const char*>& requested_extensions) {
		assert(!gpus.empty() && "No physical devices were found on the system.");

		if (selected_gpu_index >= 0) {
			EDGE_LOGI("[VkInstance] Explicitly selecting GPU {}", selected_gpu_index);
			if (selected_gpu_index > gpus.size() - 1) {
				EDGE_LOGE("[VkInstance] Selected GPU index is not within no. of available GPUs");
			}
			return;
		}

		int32_t fallback_device_index{ -1 };
		for (int32_t device_index = 0; device_index < gpus.size(); ++device_index) {
			auto& device = gpus[device_index];
			auto& properties = device->get_properties();

			if (properties.apiVersion < api_version) {
				EDGE_LOGW("[VkInstance] Device is not supported. Required API version: {}, but device supporting {}.", api_version, properties.apiVersion);
				continue;
			}

			// Device should support all required extensions
			for (const auto& extension_name : requested_extensions) {
				if (!device->is_extension_supported(extension_name)) {
					EDGE_LOGI("[VkInstance] Device {} is not supporting required extension: {}.", properties.deviceName.data(), extension_name);
				}
			}

			// Discrete gpu is in higher priority
			if (properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) {
				fallback_device_index = device_index;
				continue;
			}

			// Check that any device queue family is support present
			if (surface) {
				bool support_present{ false };

				auto family_properties = device->get_queue_family_properties();
				for (int32_t index = 0; index < family_properties.size(); ++index) {
					if (device->get_handle().getSurfaceSupportKHR(index, surface)) {
						support_present = true;
						break;
					}
				}

				if (!support_present)
					continue;
			}

			selected_gpu_index = device_index;
		}

		if (selected_gpu_index < 0) {
			if (fallback_device_index < 0) {
				EDGE_LOGE("[VkInstance] Couldn't find a physical device.");
			}

			selected_gpu_index = fallback_device_index;
		}
	}

	auto Instance::is_enabled(const char* extension) const noexcept -> bool {
		return std::find_if(enabled_extensions.begin(), enabled_extensions.end(), [extension](const char* enabled_extension) { return strcmp(extension, enabled_extension) == 0; }) != enabled_extensions.end();
	}

	bool Instance::enable_extension(const char* extension_name) {
		for (auto& avail_ext_it : available_extensions) {
			if (strcmp(avail_ext_it.extensionName, extension_name) == 0) {
				auto it = std::find_if(enabled_extensions.begin(), enabled_extensions.end(),
					[extension_name](const char* enabled_ext_name) {
						return strcmp(enabled_ext_name, extension_name) == 0;
					});
				if (it != enabled_extensions.end()) {
					// Extension is already enabled
				}
				else {
					EDGE_LOGI("[VkInstance] Extension {} found, enabling it", extension_name);
					enabled_extensions.emplace_back(extension_name);
				}
				return true;
			}
		}

		EDGE_LOGI("[VkInstance] Extension {} not found", extension_name);
		return false;
	}
}