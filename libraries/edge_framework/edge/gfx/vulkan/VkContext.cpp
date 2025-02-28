#include "VkContext.h"
#include "Config.h"

#include "../../util/Logger.h"
#include "../../util/Profiler.h"
#include "../../runtime/Window.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#include "../../runtime/win64/WindowsNativeSurface.h"
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include "../../runtime/android/AndroidNativeSurface.h"
#endif

#include "VkPhysicalDevice.h"
#include "VkDevice.h"
#include "VkDebug.h"

#include "VkBuffer.h"
#include "VkFence.h"
#include "VkSemaphore.h"
#include "VkSampler.h"
#include "VkPipelineCache.h"
#include "VkPipeline.h"
#include "VkCommandPool.h"
#include "VkQueryPool.h"
#include "VkTexture.h"
#include "VkSwapchain.h"
#include "VkShader.h"
#include "VkRootSignature.h"
#include "VkDescriptorPool.h"
#include "VkDescriptorAllocator.h"
#include "VkSharedDescriptor.h"

#include "VkUtil.h"

#include <volk.h>

#include <algorithm>

#if USE_NSIGHT_AFTERMATH
#include "VkNsightAftermath.h"
#endif


#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

#define SAFE_DELETE(p) if(p){ delete p; p = nullptr; }

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			template<typename _Ty, typename _KTy>
			void RemoveHandle(std::vector<_Ty>& handles, _KTy* handle)
			{
				auto to_delete = std::remove_if(handles.begin(), handles.end(), [handle](const std::unique_ptr<_Ty>& h) { return h.get() == handle; });
				handles.erase(to_delete);
			}

			VulkanContext::VulkanContext(const ContextCreateInfo& create_info)
			{
#if USE_NSIGHT_AFTERMATH
				nsight_aftermath = std::make_unique<VkNsightAftermath>();
#endif

				// Register dispatcher
				PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
				VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

				// Create allocation callbacks
				//RegisterAllocationCallbacks();

				auto* window = create_info.window;

				bool headless = window->GetWindowMode() == edge::Window::Mode::Headless;

				auto instance_builder = InstanceBuilder();
				instance_builder.SetHeadless(headless);
				instance_builder.SetApiVersion(VK_API_VERSION_1_1);
				instance_builder.SetApplicationName("App");
				instance_builder.SetEngineName("NewEngineApp");

				// for a while we're running on mixed C- and C++-bindings, needing volk for the C-bindings!
				VkResult result = volkInitialize();
				if (result)
				{
					return;
					//throw VulkanException(static_cast<vk::Result>(result), "Failed to initialize volk.");
				}

				// Apply instance extensions requested by surface
				{
					auto required_surface_extensions = window->GetRequiredSurfaceExtensions();
					instance_builder.AddRequiredExtensions(required_surface_extensions);
				}

				// Apply extensions requested by user
				instance_builder.AddRequiredExtensions(std::span<const char*>(required_instance_extensions));

#ifdef VULKAN_DEBUG
				bool VK_EXT_debug_utils_enabled{ false };
				{
					std::vector<vk::ExtensionProperties> available_instance_extensions = vk::enumerateInstanceExtensionProperties();
					auto debugExtensionIt = std::find_if(available_instance_extensions.begin(), available_instance_extensions.end(),
						[](vk::ExtensionProperties const& ep) { return strcmp(ep.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0; });
					if (debugExtensionIt != available_instance_extensions.end())
					{
						EDGE_LOGI("Vulkan debug utils enabled ({})", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
						instance_builder.AddRequiredExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
						VK_EXT_debug_utils_enabled = true;
					}
				}
#endif

				instance = instance_builder.BuildUnique();

				auto* native_surface = window->GetNativeSurface();
#if defined(VK_USE_PLATFORM_WIN32_KHR)
				{
					win32::WindowsNativeSurface* windows_surface = static_cast<win32::WindowsNativeSurface*>(native_surface);

					vk::Win32SurfaceCreateInfoKHR surface_create_info{};
					surface_create_info.hwnd = windows_surface->GetHandle();
					surface_create_info.hinstance = windows_surface->GetHandleInstance();

					auto result = Handle(*instance).createWin32SurfaceKHR(&surface_create_info, allocation_callbacks, &surface);
				}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
				{
					android::AndroidNativeSurface* android_surface = static_cast<android::AndroidNativeSurface*>(native_surface);

					vk::AndroidSurfaceCreateInfoKHR surface_create_info{};
					surface_create_info.window = android_surface->GetHandle();
					auto result = Handle(*instance).createAndroidSurfaceKHR(&surface_create_info, allocation_callbacks, &surface);
				}
#endif

				if (!surface)
				{
					throw std::runtime_error("Failed to create window surface.");
				}

				// Creating vulkan device, specifying the swapchain extension always
				if (!headless || instance->IsEnabled(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
				{
					device_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

					if (instance->IsEnabled(VK_KHR_DISPLAY_EXTENSION_NAME))
					{
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
				device_extensions.insert(device_extensions.end(), required_device_extensions, required_device_extensions + std::size(required_device_extensions));
				device_extensions.insert(device_extensions.end(), optional_device_extensions, optional_device_extensions + std::size(optional_device_extensions));

				// Pick best device from found on device
				instance->SelectBestDevice(surface, device_extensions);

				auto& physical_device = instance->GetBestDevice();

#ifdef VULKAN_DEBUG
				bool VK_EXT_debug_marker_enabled{ false };
				if (!VK_EXT_debug_utils_enabled)
				{
					std::vector<vk::ExtensionProperties> available_device_extensions = Handle(physical_device).enumerateDeviceExtensionProperties();
					auto debugExtensionIt = std::find_if(available_device_extensions.begin(), available_device_extensions.end(),
						[](vk::ExtensionProperties const& ep) { return strcmp(ep.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0; });
					if (debugExtensionIt != available_device_extensions.end())
					{
						EDGE_LOGI("Vulkan debug utils enabled ({})", VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
						device_extensions.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
						VK_EXT_debug_marker_enabled = true;
					}
				}
#endif

				// Enable vulkan features
				EnableDeviceFeatures(physical_device);

				// Enable vk 1.0 features
				auto& requested_features = physical_device.GetMutableRequestedFeatures();
				EnablePhysicalDeviceFeatures(requested_features);

				device = std::make_unique<Device>(physical_device, surface, device_extensions);

				// Create debug utils
				if (VK_EXT_debug_utils_enabled)
				{
					debug_utils = std::make_unique<DebugUtilsExtDebugUtils>(*device);
				}
				else if (VK_EXT_debug_marker_enabled)
				{
					debug_utils = std::make_unique<DebugMarkerExtDebugUtils>(*device);
				}
				else
				{
					debug_utils = std::make_unique<DummyDebugUtils>(*device);
				}

				device->SetDebugUtils(debug_utils.get());

				allocator = std::make_unique<Allocator>(*device);
			}

			VulkanContext::~VulkanContext()
			{
#if USE_NSIGHT_AFTERMATH
				nsight_aftermath.reset();
#endif
				Handle(*instance).destroySurfaceKHR(surface, allocation_callbacks);

				if (allocation_callbacks)
				{
					delete allocation_callbacks;
					allocation_callbacks = nullptr;
				}
			}

			void VulkanContext::Destroy(IGFXResource* handle) const
			{
				SAFE_DELETE(handle);
			}

			auto VulkanContext::CreateFence() const -> IFence*
			{
				return new VulkanFence(*device);
			}

			auto VulkanContext::CreateSemaphore() const -> ISemaphore*
			{
				return new VulkanSemaphore(*device);
			}

			auto VulkanContext::RequestQueue(QueueTypeFlags include_queue, QueueTypeFlags exclude_queue) const -> IQueue*
			{
				return device->RequestQueue(include_queue, exclude_queue);
			}

			auto VulkanContext::CreateSwapchain(const SwapChainCreateInfo& create_info) const -> ISwapchain*
			{
#ifdef VK_USE_PLATFORM_ANDROID_KHR
				vk::PresentModeKHR present_mode = (create_info.window->GetProperties().vsync == Window::Vsync::OFF) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
				std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate };
#else
				vk::PresentModeKHR present_mode = (create_info.window->GetProperties().vsync == edge::Window::Vsync::ON) ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
				std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eImmediate };
#endif
				if (create_info.old_swapchain)
				{
					auto* old_swapchain = static_cast<VulkanSwapchain*>(create_info.old_swapchain);

					// Recreate swapchain with changed size
					if (create_info.width != 1u && create_info.height != 1u)
					{
						return new VulkanSwapchain(*old_swapchain, vk::Extent2D{ create_info.width, create_info.height });
					}
					// Recreate with new amount of images
					else if (create_info.image_count != 1u && create_info.image_count != old_swapchain->GetImageCount())
					{
						return new VulkanSwapchain(*old_swapchain, create_info.image_count);
					}
				}

				return new VulkanSwapchain(*device,
					surface,
					present_mode,
					present_mode_priority_list,
					surface_priority_list,
					vk::Extent2D{ create_info.width, create_info.height },
					create_info.image_count);
			}

			auto VulkanContext::CreateCommandPool(const IQueue* queue, CommandPoolResetMode reset_mode) const -> ICommandPool*
			{
				auto* vkqueue = static_cast<const VulkanQueue*>(queue);
				return new VulkanCommandPool(*vkqueue, reset_mode);
			}

			auto VulkanContext::CreateBuffer(const BufferCreateInfo& create_info) const -> IBuffer*
			{
				return new VulkanBuffer(*allocator, create_info);
			}

			auto VulkanContext::CreateTexture(const TextureCreateInfo& create_info) const -> ITexture*
			{
				return new VulkanTexture(*allocator, create_info);
			}

			auto VulkanContext::CreateSwapchainTexture(ISwapchain* swapchain, uint32_t image_index) const -> ITexture*
			{
				return new VulkanTexture(*allocator, swapchain, image_index);
			}

			auto VulkanContext::CreateShader(std::span<const ShaderStageInfo> shader_stages) const -> IShader*
			{
				return new VulkanShader(*device, shader_stages);
			}

			auto VulkanContext::CreateRootSignature(const RootSignatureCreateInfo& create_info) const -> IRootSignature*
			{
				return new VulkanRootSignature(*device, create_info);
			}

			auto VulkanContext::CreatePipeline(const GraphicsPipelineCreateInfo& create_info) const -> IPipeline*
			{
				return new VulkanGraphicsPipeline(*device, create_info);
			}

			auto VulkanContext::CreatePipeline(const ComputePipelineCreateInfo& create_info) const -> IPipeline*
			{
				return new VulkanComputePipeline(*device, create_info);
			}

			auto VulkanContext::CreatePipelineCache(const PipelineCacheCreateInfo& create_info) const -> IPipelineCache*
			{
				return new VulkanPipelineCache(*device, create_info);
			}

			auto VulkanContext::CreateDescriptorAllocator() const -> IDescriptorAllocator*
			{
				return new VulkanDescriptorAllocator(*device);
			}

			auto VulkanContext::CreateSharedDescriptor(const SharedDescriptorCreateInfo& create_info) const -> ISharedDescriptor*
			{
				return new VulkanSharedDescriptor(*device, create_info);
			}

			auto VulkanContext::CreateSampler(const SamplerCreateInfo& create_info) const -> ISampler*
			{
				return new VulkanSampler(*device, create_info);
			}

			auto VulkanContext::CreateQueryPool(const QueryPoolCreateInfo& create_info) const -> IQueryPool*
			{
				return new VulkanQueryPool(*device, create_info);
			}

			void VulkanContext::WaitForFences(std::span<IFence*> fences, uint64_t timeout) const
			{
				std::array<vk::Fence, 16ull> wait_fences{};

				for (int32_t i = 0; i < fences.size(); ++i)
				{
					wait_fences[i] = static_cast<VulkanFence*>(fences[i])->GetHandle();
				}

				vk::Result result = Handle(*device).waitForFences(static_cast<uint32_t>(fences.size()), wait_fences.data(), VK_TRUE, timeout);
			}

			void VulkanContext::RequestMinimalBufferAlignment(BufferUsageFlags usage, uint64_t& alignment) const
			{
				alignment = 1ull;

				auto& physical = device->GetGpu();
				auto& properties = physical.GetProperties();

				if (usage & BufferUsageFlag::eVertex)
				{
					alignment = std::max<uint64_t>(alignment, 16ull);
				}

				if (usage & BufferUsageFlag::eIndex)
				{
					alignment = std::max<uint64_t>(alignment, 4ull);
				}

				if (usage & BufferUsageFlag::eUniform)
				{
					alignment = std::lcm(properties.limits.minUniformBufferOffsetAlignment, properties.limits.nonCoherentAtomSize);
				}

				if (usage & BufferUsageFlag::eStorage)
				{
					alignment = std::max(alignment, properties.limits.minStorageBufferOffsetAlignment);
				}
			}

			void VulkanContext::RequestTimestampFrequency(float* frequency) const
			{
				auto& physical = instance->GetBestDevice();
				auto& properties = physical.GetProperties();
				*frequency = properties.limits.timestampPeriod;
			}

			void VulkanContext::RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory) const
			{
				vk::MemoryHeapFlags memory_flags{ vk::MemoryHeapFlagBits::eDeviceLocal };
				vk::PhysicalDeviceMemoryBudgetPropertiesEXT memory_budget_props{};
				vk::PhysicalDeviceMemoryProperties2 memory_props{};
				memory_props.pNext = &memory_budget_props;

				auto& physical_device = instance->GetBestDevice();
				Handle(physical_device).getMemoryProperties2(&memory_props);

				*total_memory = 0ull;
				*used_memory = 0ull;

				auto heaps_count = static_cast<uint64_t>(memory_props.memoryProperties.memoryHeapCount);
				for (int32_t i = 0; i < heaps_count; ++i)
				{
					auto heapFlags = memory_props.memoryProperties.memoryHeaps[i].flags;
					if (heapFlags & memory_flags)
					{
						*total_memory += memory_budget_props.heapBudget[i];
						*used_memory += memory_budget_props.heapUsage[i];
					}
				}
			}

			void VulkanContext::EnableDeviceFeatures(PhysicalDevice& device)
			{
				for (auto& extension_name : device_extensions)
				{
					if (strcmp(extension_name, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceSynchronization2FeaturesKHR>();
					else if (strcmp(extension_name, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceTimelineSemaphoreFeaturesKHR>();
					else if (strcmp(extension_name, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceDynamicRenderingFeaturesKHR>();
					else if (strcmp(extension_name, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR>();
					else if (strcmp(extension_name, VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT>();
					else if (strcmp(extension_name, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
					else if (strcmp(extension_name, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>();
					else if (strcmp(extension_name, VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR>();
					else if (strcmp(extension_name, VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceDescriptorBufferFeaturesEXT>();
					else if (strcmp(extension_name, VK_EXT_SHADER_OBJECT_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceShaderObjectFeaturesEXT>();
					else if (strcmp(extension_name, VK_KHR_8BIT_STORAGE_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDevice8BitStorageFeaturesKHR>();
					else if (strcmp(extension_name, VK_KHR_16BIT_STORAGE_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDevice16BitStorageFeaturesKHR>();
					else if (strcmp(extension_name, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceShaderFloat16Int8FeaturesKHR>();
					//else if (strcmp(extension_name, VK_NV_MESH_SHADER_EXTENSION_NAME) == 0)
					//	device.RequestExtensionFeatures<vk::PhysicalDeviceMeshShaderPropertiesEXT>();
					else if (strcmp(extension_name, VK_EXT_MULTI_DRAW_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceMultiDrawFeaturesEXT>();
					else if (strcmp(extension_name, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceHostQueryResetFeaturesEXT>();
					else if (strcmp(extension_name, VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0)
					{
						auto& ext = device.RequestExtensionFeatures<vk::PhysicalDeviceMeshShaderFeaturesEXT>();
						ext.multiviewMeshShader = VK_FALSE;
						ext.primitiveFragmentShadingRateMeshShader = VK_FALSE;
					}
					else if (strcmp(extension_name, VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceDiagnosticsConfigFeaturesNV>();
					else if (strcmp(extension_name, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
					else if (strcmp(extension_name, VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME) == 0)
						device.RequestExtensionFeatures<vk::PhysicalDeviceShaderTileImageFeaturesEXT>();
					else if (strcmp(extension_name, VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME) == 0)
					{
						auto& ext = device.RequestExtensionFeatures<vk::PhysicalDeviceVulkanMemoryModelFeatures>();
						ext.vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE;
					}
					else if (strcmp(extension_name, VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME) == 0)
						auto& ext = device.RequestExtensionFeatures<vk::PhysicalDeviceDiagnosticsConfigFeaturesNV>();
					//else if(strcmp(extension_name, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME) == 0)
					//	device.RequestExtensionFeatures(vk::PhysicalDeviceIndirect)
				}
			}
		}
	}
}