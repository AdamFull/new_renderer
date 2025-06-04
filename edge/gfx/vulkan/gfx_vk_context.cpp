#include "gfx_vk_context.h"
#include "gfx_vk_config.h"

#include "../../util/Logger.h"
#include "../../util/utl_profiler.h"
#include "../../runtime/rt_window.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#include "../../runtime/win64/rt_windows_native_surface.h"
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include "../../runtime/android/rt_android_native_surface.h"
#endif

#include "gfx_vk_physical_device.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"

#include "gfx_vk_acceleration_structure.h"
#include "gfx_vk_buffer.h"
#include "gfx_vk_fence.h"
#include "gfx_vk_semaphore.h"
#include "gfx_vk_sampler.h"
#include "gfx_vk_pipeline_cache.h"
#include "gfx_vk_pipeline.h"
#include "gfx_vk_command_pool.h"
#include "gfx_vk_query_pool.h"
#include "gfx_vk_texture.h"
#include "gfx_vk_swapchain.h"
#include "gfx_vk_shader.h"
#include "gfx_vk_root_signature.h"
#include "gfx_vk_descriptor_allocator.h"
#include "gfx_vk_descriptor_binding_layout.h"

#include "gfx_vk_util.h"

#include <volk.h>

#include <algorithm>

#if USE_NSIGHT_AFTERMATH
#include "gfx_vk_nsight_aftermath.h"
#endif

#define SAFE_DELETE(p) if(p){ delete p; p = nullptr; }

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace edge::gfx::vulkan {
	GFXContext::GFXContext(const ContextCreateInfo& create_info) {
#if USE_NSIGHT_AFTERMATH
		nsight_aftermath = std::make_unique<VkNsightAftermath>();
#endif

		// Register dispatcher
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		// Create allocation callbacks
		//RegisterAllocationCallbacks();

		auto* window = create_info.window;

		bool headless = window->get_window_mode() == edge::Window::Mode::Headless;

		auto instance_builder = InstanceBuilder();
		instance_builder.set_headless(headless);
		instance_builder.set_api_version(VK_API_VERSION_1_2);
		instance_builder.set_application_name("App");
		instance_builder.set_engine_name("NewEngineApp");

		// for a while we're running on mixed C- and C++-bindings, needing volk for the C-bindings!
		VkResult result = volkInitialize();
		if (result) {
			EDGE_LOGE("[VkContext] Failed to initialize volk.");
		}

		// Apply instance extensions requested by surface
		{
			auto required_surface_extensions = window->get_required_surface_extensions();
			instance_builder.add_required_extensions(required_surface_extensions);
		}

		// Apply surface extension if we use it
		if (!headless) {
			instance_builder.add_required_extension(VK_KHR_SURFACE_EXTENSION_NAME);
		}

#ifdef VULKAN_DEBUG
		bool VK_EXT_debug_utils_enabled{ false };
		{
			std::vector<vk::ExtensionProperties> available_instance_extensions = vk::enumerateInstanceExtensionProperties();
			auto debugExtensionIt = std::find_if(available_instance_extensions.begin(), available_instance_extensions.end(),
				[](vk::ExtensionProperties const& ep) { return strcmp(ep.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0; });
			if (debugExtensionIt != available_instance_extensions.end()) {
				EDGE_LOGI("[VkContext] Vulkan debug utils enabled ({})", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				instance_builder.add_required_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				VK_EXT_debug_utils_enabled = true;
			}
		}
#endif

		instance = instance_builder.build_unique();

		auto* native_surface = window->get_native_surface();
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		{
			win64::WindowsNativeSurface* windows_surface = utl::cast<win64::WindowsNativeSurface*>(native_surface);

			vk::Win32SurfaceCreateInfoKHR surface_create_info{};
			surface_create_info.hwnd = windows_surface->get_handle();
			surface_create_info.hinstance = windows_surface->get_handle_instance();

			auto result = instance->get_handle().createWin32SurfaceKHR(&surface_create_info, allocation_callbacks, &surface);
		}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		{
			android::AndroidNativeSurface* android_surface = utl::cast<android::AndroidNativeSurface*>(native_surface);

			vk::AndroidSurfaceCreateInfoKHR surface_create_info{};
			surface_create_info.window = android_surface->get_handle();
			auto result = instance->get_handle().createAndroidSurfaceKHR(&surface_create_info, allocation_callbacks, &surface);
		}
#endif

		if (!surface) {
			EDGE_LOGE("[VkContext] Failed to create window surface.");
		}

		// Creating vulkan device, specifying the swapchain extension always
		if (!headless || instance->is_enabled(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME)) {
			device_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			if (instance->is_enabled(VK_KHR_DISPLAY_EXTENSION_NAME)) {
				// Optional
				device_extensions.emplace_back(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME);
			}
		}

#ifdef VULKAN_ENABLE_PORTABILITY
		// VK_KHR_portability_subset must be enabled if present in the implementation (e.g on macOS/iOS with beta extensions enabled)
		// Optional
		AddDeviceExtension(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

		// Apply required device extensions
		device_extensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

		// Enable extensions for use nsight aftermath
#if USE_NSIGHT_AFTERMATH
		device_extensions.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
		device_extensions.push_back(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME);
#endif

		// Optional extensions
		if (!headless) {
			device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}

		if (create_info.require_features.ray_tracing) {
			device_extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
			device_extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
			device_extensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
			device_extensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
		}

		device_extensions.insert(device_extensions.end(), k_required_device_extensions, k_required_device_extensions + std::size(k_required_device_extensions));

		// Pick best device from found on device
		instance->select_best_device(surface, device_extensions);

		auto& physical_device = instance->get_best_device();

#ifdef VULKAN_DEBUG
		bool VK_EXT_debug_marker_enabled{ false };
		if (!VK_EXT_debug_utils_enabled) {
			std::vector<vk::ExtensionProperties> available_device_extensions = physical_device.get_handle().enumerateDeviceExtensionProperties();
			auto debugExtensionIt = std::find_if(available_device_extensions.begin(), available_device_extensions.end(),
				[](vk::ExtensionProperties const& ep) { return strcmp(ep.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0; });
			if (debugExtensionIt != available_device_extensions.end()) {
				EDGE_LOGI("[VkContext] Vulkan debug utils enabled ({})", VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
				device_extensions.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
				VK_EXT_debug_marker_enabled = true;
			}
		}
#endif

		// Enable vulkan features
		enable_device_features(physical_device);

		// TODO: validate required features
		//features.samplerAnisotropy = true;
		//features.sampleRateShading = true;
		//features.fillModeNonSolid = true;
		//features.multiViewport = true;
		//features.geometryShader = true;
		//features.tessellationShader = true;
		//features.fragmentStoresAndAtomics = true;
		//features.logicOp = true;
		//features.imageCubeArray = true;
		//features.depthClamp = true;
		//features.independentBlend = true;
		//features.multiDrawIndirect = true;
		//features.shaderInt16 = true;
		//features.shaderStorageImageReadWithoutFormat = true;
		//features.shaderStorageImageWriteWithoutFormat = true;
		//features.shaderInt64 = true;

		//vk::PhysicalDeviceVulkan12Features vk12_features{};
		//vk12_features.storageBuffer8BitAccess = true;
		//vk12_features.uniformAndStorageBuffer8BitAccess = true;
		//vk12_features.shaderFloat16 = true;
		//vk12_features.shaderInt8 = true;
		//vk12_features.descriptorBindingUniformBufferUpdateAfterBind = true;
		//vk12_features.descriptorBindingSampledImageUpdateAfterBind = true;
		//vk12_features.descriptorBindingStorageImageUpdateAfterBind = true;
		//vk12_features.descriptorBindingStorageBufferUpdateAfterBind = true;
		//vk12_features.descriptorBindingUniformTexelBufferUpdateAfterBind = true;
		//vk12_features.descriptorBindingStorageTexelBufferUpdateAfterBind = true;
		//vk12_features.descriptorBindingUpdateUnusedWhilePending = true;
		//vk12_features.descriptorBindingPartiallyBound = true;
		//vk12_features.descriptorBindingVariableDescriptorCount = true;
		//vk12_features.hostQueryReset = true;
		//vk12_features.timelineSemaphore = true;
		//vk12_features.bufferDeviceAddress = true;
		//vk12_features.bufferDeviceAddressCaptureReplay = true;
		//vk12_features.bufferDeviceAddressMultiDevice = true;
		//vk12_features.vulkanMemoryModel = true;
		//vk12_features.vulkanMemoryModelDeviceScope = true;

		device = std::make_unique<Device>(physical_device, surface, device_extensions);

		// Create debug utils
		if (VK_EXT_debug_utils_enabled) {
			debug_utils = std::make_unique<DebugUtilsExtDebugUtils>(*device);
		}
		else if (VK_EXT_debug_marker_enabled) {
			debug_utils = std::make_unique<DebugMarkerExtDebugUtils>(*device);
		}
		else {
			debug_utils = std::make_unique<DummyDebugUtils>(*device);
		}

		device->set_debug_utils(debug_utils.get());

		allocator = std::make_unique<Allocator>(*device);
	}

	GFXContext::~GFXContext() {
#if USE_NSIGHT_AFTERMATH
		nsight_aftermath.reset();
#endif
		instance->get_handle().destroySurfaceKHR(surface, allocation_callbacks);

		if (allocation_callbacks) {
			delete allocation_callbacks;
			allocation_callbacks = nullptr;
		}
	}

	void GFXContext::destroy(IGFXResource* handle) const {
		SAFE_DELETE(handle);
	}

	auto GFXContext::create_fence() const -> IFence* {
		return new Fence(*device);
	}

	auto GFXContext::create_semaphore(SemaphoreType type) const -> ISemaphore* {
		return new Semaphore(*device, type == SemaphoreType::eTimeline);
	}

	auto GFXContext::get_queue_count(QueueType queue_type) const->uint32_t {
		return device->get_queue_count(queue_type);
	}

	auto GFXContext::get_queue(QueueType queue_type, uint32_t queue_index) const->IQueue const* {
		return device->get_queue(queue_type, queue_index);
	}

	auto GFXContext::create_swapchain(const SwapChainCreateInfo& create_info) const -> ISwapchain* {
#ifdef VK_USE_PLATFORM_ANDROID_KHR
		vk::PresentModeKHR present_mode = (create_info.window->get_properties().vsync == Window::Vsync::OFF) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
		std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate };
#else
		vk::PresentModeKHR present_mode = (create_info.window->get_properties().vsync == edge::Window::Vsync::ON) ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
		std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eImmediate };
#endif
		if (create_info.old_swapchain) {
			auto* old_swapchain = utl::cast<Swapchain*>(create_info.old_swapchain);

			// Recreate swapchain with changed size
			if (create_info.width != 1u && create_info.height != 1u) {
				return new Swapchain(*old_swapchain, vk::Extent2D{ create_info.width, create_info.height });
			}
			// Recreate with new amount of images
			else if (create_info.image_count != 1u && create_info.image_count != old_swapchain->get_image_count()) {
				return new Swapchain(*old_swapchain, create_info.image_count);
			}
		}

		return new Swapchain(*device,
			surface,
			present_mode,
			present_mode_priority_list,
			surface_priority_list,
			vk::Extent2D{ create_info.width, create_info.height },
			create_info.image_count);
	}

	auto GFXContext::create_command_pool(const IQueue* queue, CommandPoolResetMode reset_mode) const -> ICommandPool* {
		auto* vkqueue = utl::cast<const Queue*>(queue);
		return new CommandPool(*vkqueue, reset_mode);
	}

	auto GFXContext::create_buffer(const BufferCreateInfo& create_info) const -> IBuffer*
	{
		return new Buffer(*allocator, create_info);
	}

	auto GFXContext::create_texture(const TextureCreateInfo& create_info) const -> ITexture*
	{
		return new Texture(*allocator, create_info);
	}

	auto GFXContext::create_swapchain_texture(ISwapchain* swapchain, uint32_t image_index) const -> ITexture*
	{
		return new Texture(*allocator, swapchain, image_index);
	}

	auto GFXContext::create_shader(std::span<const ShaderStageInfo> shader_stages, ShaderReflection* refl) const -> IShader*
	{
		return new Shader(*device, shader_stages, refl);
	}

	auto GFXContext::create_root_signature(const RootSignatureCreateInfo& create_info) const -> IRootSignature*
	{
		return new RootSignature(*device, create_info);
	}

	auto GFXContext::create_pipeline(const GraphicsPipelineCreateInfo& create_info) const -> IPipeline*
	{
		return new GraphicsPipeline(*device, create_info);
	}

	auto GFXContext::create_pipeline(const ComputePipelineCreateInfo& create_info) const -> IPipeline*
	{
		return new ComputePipeline(*device, create_info);
	}

	auto GFXContext::create_pipeline_cache(const PipelineCacheCreateInfo& create_info) const -> IPipelineCache*
	{
		return new PipelineCache(*device, create_info);
	}

	auto GFXContext::create_descriptor_allocator() const -> IDescriptorAllocator*
	{
		return new DescriptorAllocator(*device);
	}

	auto GFXContext::create_descriptor_layout(const DescriptorLayoutCreateInfo& create_info) const -> IDescriptorLayout* {
		return new DescriptorSetLayout(*device, create_info);
	}

	auto GFXContext::create_binding_layout(IDescriptorLayout* descriptor_layout) const->IBindingLayout* {
		return new DescriptorBindingLayout(*device, descriptor_layout);
	}

	auto GFXContext::create_sampler(const SamplerCreateInfo& create_info) const -> ISampler* {
		return new Sampler(*device, create_info);
	}

	auto GFXContext::create_query_pool(const QueryPoolCreateInfo& create_info) const -> IQueryPool* {
		return new QueryPool(*device, create_info);
	}

	auto GFXContext::create_acceleration_structure(const AccelerationStructureCreateInfo& create_info) const -> IAccelerationStructure* {
		return new AccelerationStructure(*device, create_info);
	}

	void GFXContext::wait_for_fences(std::span<IFence*> fences, uint64_t timeout) const {
		utl::fixed_vector<vk::Fence, 16ull> wait_fences{};

		for (auto& fence : fences) {
			wait_fences.push_back(utl::cast<Fence*>(fence)->get_handle());
		}

		vk::Result result = device->get_handle().waitForFences(static_cast<uint32_t>(fences.size()), wait_fences.data(), VK_TRUE, timeout);
	}

	void GFXContext::request_minimal_buffer_alignment(BufferUsageFlags usage, uint64_t& alignment) const {
		alignment = 1ull;

		auto& physical = device->get_gpu();
		auto& properties = physical.get_properties();

		if (usage & BufferUsageFlag::eVertex) {
			alignment = std::max<uint64_t>(alignment, 16ull);
		}

		if (usage & BufferUsageFlag::eIndex) {
			alignment = std::max<uint64_t>(alignment, 4ull);
		}

		if (usage & BufferUsageFlag::eUniform) {
			alignment = std::lcm(properties.limits.minUniformBufferOffsetAlignment, properties.limits.nonCoherentAtomSize);
		}

		if (usage & BufferUsageFlag::eStorage) {
			alignment = std::max(alignment, properties.limits.minStorageBufferOffsetAlignment);
		}
	}

	void GFXContext::request_timestamp_frequency(double* frequency) const {
		auto& physical = instance->get_best_device();
		auto& properties = physical.get_properties();
		*frequency = (double)properties.limits.timestampPeriod;
	}

	void GFXContext::request_memory_stats(MemoryUsageStats& memory_stats) const {
		vk::PhysicalDeviceMemoryBudgetPropertiesEXT memory_budget_props{};
		vk::PhysicalDeviceMemoryProperties2 memory_props{};
		memory_props.pNext = &memory_budget_props;

		auto& physical_device = instance->get_best_device();
		physical_device.get_handle().getMemoryProperties2(&memory_props);

		static std::string_view dedicated_vram_str{ "Dedicated VRAM" };
		static std::string_view shared_system_str{ "Shared System" };

		auto heap_count = static_cast<uint64_t>(memory_props.memoryProperties.memoryHeapCount);
		for (int32_t i = 0; i < heap_count; ++i) {
			auto& heap = memory_props.memoryProperties.memoryHeaps[i];

			MemoryHeapInfo heap_info{};
			heap_info.name = ((heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) == vk::MemoryHeapFlagBits::eDeviceLocal) ? dedicated_vram_str : shared_system_str;
			heap_info.total_size = heap.size;
			heap_info.budget = memory_budget_props.heapBudget[i];
			heap_info.used_size = memory_budget_props.heapUsage[i];
			memory_stats.memory_heaps.push_back(heap_info);
		}
	}

	void GFXContext::enable_device_features(PhysicalDevice& device) {
		for (auto& extension_name : device_extensions) {
			if (strcmp(extension_name, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0)
				device.request_extension_features<vk::PhysicalDeviceSynchronization2FeaturesKHR>();
			else if (strcmp(extension_name, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
				device.request_extension_features<vk::PhysicalDeviceDynamicRenderingFeaturesKHR>();
			else if (strcmp(extension_name, VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME) == 0)
				device.request_extension_features<vk::PhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT>();
			else if (strcmp(extension_name, VK_KHR_16BIT_STORAGE_EXTENSION_NAME) == 0)
				device.request_extension_features<vk::PhysicalDevice16BitStorageFeaturesKHR>();
			else if (strcmp(extension_name, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0)
				device.request_extension_features<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
			else if (strcmp(extension_name, VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME) == 0)
				auto& ext = device.request_extension_features<vk::PhysicalDeviceDiagnosticsConfigFeaturesNV>();
			else if (strcmp(extension_name, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0) {
				auto& ext = device.request_extension_features<vk::PhysicalDeviceRayQueryFeaturesKHR>();
				if (!ext.rayQuery) {
					EDGE_LOGW("[Vulkan Context] Ray query extension is not supported.");
				}
			}

			// For ray tracing
			else if (strcmp(extension_name, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0) {
				device.request_extension_features<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>();
			}
			else if (strcmp(extension_name, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
				device.request_extension_features<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>();
			}
			else if (strcmp(extension_name, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0) {
				device.request_extension_features<vk::PhysicalDeviceRayQueryFeaturesKHR>();
			}
		}
	}
}