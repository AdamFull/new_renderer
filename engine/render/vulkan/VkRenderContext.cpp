#include "VkRenderContext.h"

#include <vulkan/vulkan.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

using namespace engine::graphics::vulkan;

void* vk_malloc(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	void* ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void vk_free(void* pUserData, void* pMemory)
{
	free(pMemory);
}

void* vk_realloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return realloc(pOriginal, size);
}

void vk_malloc_notification(void* pUserData, size_t  size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
}

void vk_free_notification(void* pUserData, size_t size, VkInternalAllocationType  allocationType, VkSystemAllocationScope allocationScope)
{
}

uint32_t get_api_version(ERenderApiVersion version)
{
	switch (version)
	{
	case ERenderApiVersion::eVK10:
		return VK_API_VERSION_1_0;
	case ERenderApiVersion::eVK11:
		return VK_API_VERSION_1_1;
	case ERenderApiVersion::eVK12:
		return VK_API_VERSION_1_2;
	case ERenderApiVersion::eVK13:
		return VK_API_VERSION_1_3;
	}

	return 0u;
}

// Check that second vector contains all items from first one
bool check_contains_all(const std::vector<const char*>& lhs, const std::vector<const char*>& rhs)
{
	return std::all_of(lhs.begin(), lhs.end(), [&rhs](const char* s1) { return std::find(rhs.begin(), rhs.end(), s1) != rhs.end(); });
}

bool check_validation_layers_support(const std::vector<const char*>& layers, const std::vector<vk::LayerProperties>& props)
{
	std::vector<const char*> avaliable_layers{};
	for (auto& prop : props)
		avaliable_layers.emplace_back(prop.layerName);

	return check_contains_all(layers, avaliable_layers);
}

std::vector<const char*> get_instance_extension_names()
{
#if defined( VK_USE_PLATFORM_WIN32_KHR )
	return std::vector<const char*>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
#else
	return std::vector<const char*>{};
#endif
}

void CVkRenderContext::initialize(const FRenderContextCreateInfo& context_ci)
{
	// Register dispatcher
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	// Create allocation callbacks
	register_allocation_callbacks();

	create_instance(context_ci);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

	create_surface(context_ci);
}

vk::Result CVkRenderContext::create_surface(const FRenderContextCreateInfo& context_ci)
{
	vk::Result result{ vk::Result::eErrorUnknown };
#if defined( VK_USE_PLATFORM_WIN32_KHR )
	vk::Win32SurfaceCreateInfoKHR surface_ci{};
	surface_ci.hinstance = context_ci.hinstance;
	surface_ci.hwnd = context_ci.hwnd;
	
	result = instance.createWin32SurfaceKHR(&surface_ci, allocator_callbacks, &surface);
#else
#endif;
	return result;
}

vk::Result CVkRenderContext::register_allocation_callbacks()
{
	allocator_callbacks = new vk::AllocationCallbacks();
	memset(allocator_callbacks, 0, sizeof(vk::AllocationCallbacks));

	allocator_callbacks->pfnAllocation = (PFN_vkAllocationFunction)(&vk_malloc);
	allocator_callbacks->pfnReallocation = (PFN_vkReallocationFunction)(&vk_realloc);
	allocator_callbacks->pfnFree = (PFN_vkFreeFunction)(&vk_free);
	allocator_callbacks->pfnInternalAllocation = (PFN_vkInternalAllocationNotification)(&vk_malloc_notification);
	allocator_callbacks->pfnInternalFree = (PFN_vkInternalFreeNotification)(&vk_free_notification);

	return vk::Result::eSuccess;
}

vk::Result CVkRenderContext::create_instance(const FRenderContextCreateInfo& context_ci)
{
	// Load instance layer props
	instance_layer_props = vk::enumerateInstanceLayerProperties();

	if (enable_validation && !check_validation_layers_support(context_ci.validation_layers, instance_layer_props))
	{
		// TODO: handle error
		return;
	}

	vk::ApplicationInfo app_info{};
	app_info.applicationVersion = app_info.engineVersion = app_info.apiVersion = get_api_version(context_ci.api_version);
	app_info.pApplicationName = app_info.pEngineName = context_ci.app_name.c_str();

	auto instance_extensions = get_instance_extension_names();

	vk::InstanceCreateInfo instance_ci{};
	instance_ci.pApplicationInfo = &app_info;
	instance_ci.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
	instance_ci.ppEnabledExtensionNames = instance_extensions.data();

	// Apply validation features
	std::vector<vk::ValidationFeatureEnableEXT> feature_enables{};
	vk::ValidationFeaturesEXT validation_features{};

	if (context_ci.validation_features & GPU_ASSISTED_FLAG_BIT)
		feature_enables.emplace_back(vk::ValidationFeatureEnableEXT::eGpuAssisted);
	if (context_ci.validation_features & GPU_ASSISTED_RESERVE_BINDING_SLOT_FLAG_BIT)
		feature_enables.emplace_back(vk::ValidationFeatureEnableEXT::eGpuAssistedReserveBindingSlot);
	if (context_ci.validation_features & BEST_PRACTICE_FLAG_BIT)
		feature_enables.emplace_back(vk::ValidationFeatureEnableEXT::eBestPractices);
	if (context_ci.validation_features & DEBUG_PRINTF_FLAG_BIT)
		feature_enables.emplace_back(vk::ValidationFeatureEnableEXT::eDebugPrintf);
	if (context_ci.validation_features & SYNCHRONIZATION_VALIDATION_FLAG_BIT)
		feature_enables.emplace_back(vk::ValidationFeatureEnableEXT::eSynchronizationValidation);

	validation_features.enabledValidationFeatureCount = static_cast<uint32_t>(feature_enables.size());
	validation_features.pEnabledValidationFeatures = feature_enables.data();

	if (enable_validation)
	{
		instance_ci.enabledLayerCount = static_cast<uint32_t>(context_ci.validation_layers.size());
		instance_ci.ppEnabledLayerNames = context_ci.validation_layers.data();
		instance_ci.pNext = &validation_features;
	}
	return vk::createInstance(&instance_ci, allocator_callbacks, &instance);
}