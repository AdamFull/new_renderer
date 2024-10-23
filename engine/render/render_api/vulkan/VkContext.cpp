#include <VkContext.h>
#include <Config.h>

#include <util/Logger.h>
#include <engine/runtime/Window.h>

#include <VkPhysicalDevice.h>
#include <VkDevice.h>
#include <VkDebug.h>

#include <VkCommon.h>
#include <VkUtil.h>

#include <volk.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

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

		VkGFXContext::VkGFXContext(const GFXContextCreateInfo& create_info)
		{
			// Register dispatcher
			PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
			VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

			// Create allocation callbacks
			//RegisterAllocationCallbacks();

			auto* window = create_info.window;

			bool headless = window->GetWindowMode() == engine::Window::Mode::Headless;

			auto instance_builder = InstanceBuilder();
			instance_builder.SetHeadless(headless);
			instance_builder.SetApiVersion(VK_VERSION_1_1);
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
			{
				std::vector<vk::ExtensionProperties> available_instance_extensions = vk::enumerateInstanceExtensionProperties();
				auto debugExtensionIt = std::find_if(available_instance_extensions.begin(), available_instance_extensions.end(),
					[](vk::ExtensionProperties const& ep) { return strcmp(ep.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0; });
				if (debugExtensionIt != available_instance_extensions.end())
				{
					log_info("Vulkan debug utils enabled ({})", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
					instance_builder.AddRequiredExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				}
			}
#endif

			instance = instance_builder.BuildUnique();

			surface = window->CreateSurface(instance->GetHandle(), allocation_callbacks);
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

			// Pick best device from found on device
			instance->SelectBestDevice(surface, device_extensions);

			auto& physical_device = instance->GetBestDevice();

#ifdef VULKAN_DEBUG
			if (!instance->IsEnabled(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
			{
				std::vector<vk::ExtensionProperties> available_device_extensions = Handle(physical_device).enumerateDeviceExtensionProperties();
				auto debugExtensionIt = std::find_if(available_device_extensions.begin(), available_device_extensions.end(),
					[](vk::ExtensionProperties const& ep) { return strcmp(ep.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0; });
				if (debugExtensionIt != available_device_extensions.end())
				{
					log_info("Vulkan debug utils enabled ({})", VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
					device_extensions.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
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
			if (instance->IsEnabled(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
			{
				debug_utils = std::make_unique<DebugUtilsExtDebugUtils>(*device);
			}
			else if (device->IsEnabled(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
			{
				debug_utils = std::make_unique<DebugMarkerExtDebugUtils>(*device);
			}
			else
			{
				debug_utils = std::make_unique<DummyDebugUtils>(*device);
			}

			allocator = std::make_unique<Allocator>(*device);
		}

		VkGFXContext::~VkGFXContext()
		{
			Handle(*instance).destroySurfaceKHR(surface, allocation_callbacks);

			if (allocation_callbacks)
			{
				delete allocation_callbacks;
				allocation_callbacks = nullptr;
			}
		}

		Fence* VkGFXContext::CreateFence()
		{
			auto& new_fence = fences.emplace_back(std::make_unique<VulkanFence>(*device));
			return new_fence.get();
		}

		void VkGFXContext::DestroyFence(Fence* fence)
		{
			auto to_delete = std::remove_if(fences.begin(), fences.end(), [fence](const std::unique_ptr<VulkanFence>& f) { return f.get() == fence; });
			fences.erase(to_delete);
		}

		Semaphore* VkGFXContext::CreateSemaphore()
		{
			auto& new_semaphore = semaphores.emplace_back(std::make_unique<VulkanSemaphore>(*device));
			return new_semaphore.get();
		}

		void VkGFXContext::DestroySemaphore(Semaphore* semaphore)
		{
			auto to_delete = std::remove_if(semaphores.begin(), semaphores.end(), [semaphore](const std::unique_ptr<VulkanSemaphore>& s) { return s.get() == semaphore; });
			semaphores.erase(to_delete);
		}

		Queue* VkGFXContext::RequestQueue(QueueTypeFlags queue_type_flags, uint32_t queue_index)
		{
			Queue* requested_queue{ nullptr };
			if (queue_type_flags & QueueTypeFlag::ePresent)
			{
				requested_queue = &device->GetQueueByPresent(queue_index);
			}
			else
			{
				vk::QueueFlags queue_flags{};
				if (queue_type_flags & QueueTypeFlag::eGraphics)
				{
					queue_flags |= vk::QueueFlagBits::eGraphics;
				}
				if (queue_type_flags & QueueTypeFlag::eCompute)
				{
					queue_flags |= vk::QueueFlagBits::eCompute;
				}
				if (queue_type_flags & QueueTypeFlag::eTransfer)
				{
					queue_flags |= vk::QueueFlagBits::eTransfer;
				}

				requested_queue = &device->GetQueueByFlags(queue_flags, queue_index);
			}
				
			return requested_queue;
		}

		Swapchain* VkGFXContext::CreateSwapchain(const SwapChainCreateInfo& create_info)
		{
#ifdef VK_USE_PLATFORM_ANDROID_KHR
			vk::PresentModeKHR present_mode = (window->GetProperties().vsync == Window::Vsync::OFF) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
			std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate };
#else
			vk::PresentModeKHR present_mode = (create_info.window->GetProperties().vsync == engine::Window::Vsync::ON) ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
			std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eImmediate };
#endif

			if (create_info.old_swapchain)
			{
				auto* old_swapchain = static_cast<VulkanSwapchain*>(create_info.old_swapchain);

				// Recreate swapchain with changed size
				if (create_info.width != 1u && create_info.height != 1u)
				{
					auto& new_swapchain = swapchains.emplace_back(std::make_unique<VulkanSwapchain>(*old_swapchain, vk::Extent2D{ create_info.width, create_info.height }));
					return new_swapchain.get();
				}
				// Recreate with new amount of images
				else if (create_info.image_count != 1u && create_info.image_count != old_swapchain->GetImageCount())
				{
					auto& new_swapchain = swapchains.emplace_back(std::make_unique<VulkanSwapchain>(*old_swapchain, create_info.image_count));
					return new_swapchain.get();
				}
			}

			auto& new_swapchain = swapchains.emplace_back(std::make_unique<VulkanSwapchain>(*device, 
				surface, 
				present_mode, 
				present_mode_priority_list, 
				surface_priority_list,
				vk::Extent2D{ create_info.width, create_info.height },
				create_info.image_count));

			return new_swapchain.get();
		}

		void VkGFXContext::DestroySwapchain(Swapchain* swapchain)
		{
			auto to_delete = std::remove_if(swapchains.begin(), swapchains.end(), [swapchain](const std::unique_ptr<VulkanSwapchain>& h) { return h.get() == swapchain; });
			swapchains.erase(to_delete);
		}

		CommandPool* VkGFXContext::CreateCommandPool(Queue* queue, CommandPoolResetMode reset_mode)
		{
			auto* vkqueue = static_cast<VulkanQueue*>(queue);
			auto& new_command_pool = command_pools.emplace_back(std::make_unique<VulkanCommandPool>(*vkqueue, reset_mode));
			return new_command_pool.get();
		}

		void VkGFXContext::DestroyCommandPool(CommandPool* command_pool)
		{
			auto to_delete = std::remove_if(command_pools.begin(), command_pools.end(), [command_pool](const std::unique_ptr<VulkanCommandPool>& h) { return h.get() == command_pool; });
			command_pools.erase(to_delete);
		}

		Buffer* VkGFXContext::CreateBuffer(const BufferCreateInfo& create_info)
		{
			auto& new_buffer = buffers.emplace_back(std::make_unique<VulkanBuffer>(*allocator, create_info));
			return new_buffer.get();
		}

		void VkGFXContext::DestroyBuffer(Buffer* buffer)
		{
			auto to_delete = std::remove_if(buffers.begin(), buffers.end(), [buffer](const std::unique_ptr<VulkanBuffer>& h) { return h.get() == buffer; });
			buffers.erase(to_delete);
		}

		Texture* VkGFXContext::CreateTexture(const TextureCreateInfo& create_info)
		{
			auto& new_texture = textures.emplace_back(std::make_unique<VulkanTexture>(*allocator, create_info));
			return new_texture.get();
		}

		Texture* VkGFXContext::CreateSwapchainTexture(Swapchain* swapchain, uint32_t image_index)
		{
			auto& new_texture = textures.emplace_back(std::make_unique<VulkanTexture>(*allocator, swapchain, image_index));
			return new_texture.get();
		}

		void VkGFXContext::DestroyTexture(Texture* texture)
		{
			auto to_delete = std::remove_if(textures.begin(), textures.end(), [texture](const std::unique_ptr<VulkanTexture>& h) { return h.get() == texture; });
			textures.erase(to_delete);
		}

		Shader* VkGFXContext::CreateShader(const ShaderCreateInfo& create_info)
		{
			auto& new_shader = shaders.emplace_back(std::make_unique<VulkanShader>(*device, create_info));
			return new_shader.get();
		}

		void VkGFXContext::DestroyShader(Shader* shader)
		{
			auto to_delete = std::remove_if(shaders.begin(), shaders.end(), [shader](const std::unique_ptr<VulkanShader>& h) { return h.get() == shader; });
			shaders.erase(to_delete);
		}

		RootSignature* VkGFXContext::CreateRootSignature(const RootSignatureCreateInfo& create_info)
		{
			assert(false && "NOT IMPLEMENTED");
			return nullptr;
		}

		void VkGFXContext::DestroyRootSignature(RootSignature* root_signature)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		Pipeline* VkGFXContext::CreatePipeline(const GraphicsPipelineCreateInfo& create_info)
		{
			assert(false && "NOT IMPLEMENTED");
			return nullptr;
		}

		Pipeline* VkGFXContext::CreatePipeline(const ComputePipelineCreateInfo& create_info)
		{
			assert(false && "NOT IMPLEMENTED");
			return nullptr;
		}

		void VkGFXContext::DestroyPipeline(Pipeline* pipeline)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		PipelineCache* VkGFXContext::CreatePipelineCache(const PipelineCacheCreateInfo& create_info)
		{
			auto& new_pipeline_cache = pipeline_caches.emplace_back(std::make_unique<VulkanPipelineCache>(*device, create_info));
			return new_pipeline_cache.get();
		}

		void VkGFXContext::DestroyPipelineCache(PipelineCache* pipeline_cache)
		{
			auto to_delete = std::remove_if(pipeline_caches.begin(), pipeline_caches.end(), [pipeline_cache](const std::unique_ptr<VulkanPipelineCache>& pc) { return pc.get() == pipeline_cache; });
			pipeline_caches.erase(to_delete);
		}

		DescriptorPool* VkGFXContext::CreateDescriptorPool()
		{
			assert(false && "NOT IMPLEMENTED");
			return nullptr;
		}

		void VkGFXContext::DestroyDescriptorPool(DescriptorPool* descriptor_pool)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		Sampler* VkGFXContext::CreateSampler(const SamplerCreateInfo& create_info)
		{
			auto& new_sampler = samplers.emplace_back(std::make_unique<VulkanSampler>(*device, create_info));
			return new_sampler.get();
		}

		void VkGFXContext::DestroySampler(Sampler* sampler)
		{
			auto to_delete = std::remove_if(samplers.begin(), samplers.end(), [sampler](const std::unique_ptr<VulkanSampler>& s) { return s.get() == sampler; });
			samplers.erase(to_delete);
		}

		QueryPool* VkGFXContext::CreateQueryPool(const QueryPoolCreateInfo& create_info)
		{
			auto& new_query_pool = query_pools.emplace_back(std::make_unique<VulkanQueryPool>(*device, create_info));
			return new_query_pool.get();
		}

		void VkGFXContext::DestroyQueryPool(QueryPool* query_pool)
		{
			auto to_delete = std::remove_if(query_pools.begin(), query_pools.end(), [query_pool](const std::unique_ptr<VulkanQueryPool>& h) { return h.get() == query_pool; });
			query_pools.erase(to_delete);
		}

		void VkGFXContext::WaitForFences(uint32_t fence_count, Fence** fences_array, uint64_t timeout)
		{
			std::array<vk::Fence, 16ull> wait_fences{};

			for (int32_t i = 0; i < fence_count; ++i)
			{
				wait_fences[i] = static_cast<VulkanFence*>(fences_array[i])->GetHandle();
			}

			vk::Result result = Handle(*device).waitForFences(fence_count, wait_fences.data(), VK_TRUE, timeout);
		}

		void VkGFXContext::RequestTimestampFrequency(double* frequency)
		{
			auto& physical = instance->GetBestDevice();
			auto& properties = physical.GetProperties();
			*frequency = static_cast<double>(properties.limits.timestampPeriod);
		}

		void VkGFXContext::RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory)
		{
			vk::MemoryHeapFlags memory_flags{ vk::MemoryHeapFlagBits::eDeviceLocal };
			vk::PhysicalDeviceMemoryBudgetPropertiesEXT memory_budget_props{};
			vk::PhysicalDeviceMemoryProperties2 memory_props{};
			memory_props.pNext = &memory_budget_props;

			auto& physical_device = instance->GetBestDevice();
			Handle(physical_device).getMemoryProperties2(&memory_props);

			*total_memory = 0ull;
			*used_memory = 0ull;

			auto heaps_count = static_cast<u64>(memory_props.memoryProperties.memoryHeapCount);
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

		void VkGFXContext::SetResourceName(GFXResource* resource_handle, const char* name)
		{
			switch (resource_handle->GetResourceType())
			{
			case ResourceType::eFence: 
			{
				auto* handle = static_cast<VulkanFence*>(resource_handle);
				debug_utils->SetDebugName(handle->GetHandle(), name);
			} break;
			case ResourceType::eSemaphore: 
			{
				auto* handle = static_cast<VulkanSemaphore*>(resource_handle);
				debug_utils->SetDebugName(handle->GetHandle(), name);
			} break;
			case ResourceType::eQueue: 
			{
				auto* handle = static_cast<VulkanQueue*>(resource_handle);
				debug_utils->SetDebugName(handle->GetHandle(), name);
			} break;
			case ResourceType::eSwapchain:
			{
				assert(false && "NOT IMPLEMENTED");
			} break;
			case ResourceType::eBuffer:
			{
				auto* handle = static_cast<VulkanBuffer*>(resource_handle);
				debug_utils->SetDebugName(handle->GetHandle(), name);
			} break;
			case ResourceType::eCommandList:
			{
				auto* handle = static_cast<VulkanCommandList*>(resource_handle);
				debug_utils->SetDebugName(handle->GetHandle(), name);
			} break;
			case ResourceType::eCommandPool:
			{
				auto* handle = static_cast<VulkanCommandPool*>(resource_handle);
				debug_utils->SetDebugName(handle->GetHandle(), name);
			} break;
			case ResourceType::eTexture:
			{
				assert(false && "NOT IMPLEMENTED");
			} break;
			case ResourceType::eSampler:
			{
				auto* handle = static_cast<VulkanSampler*>(resource_handle);
				debug_utils->SetDebugName(handle->GetHandle(), name);
			} break;
			case ResourceType::eShader:
			{
				assert(false && "NOT IMPLEMENTED");
			} break;
			case ResourceType::eRootSignature:
			{
				assert(false && "NOT IMPLEMENTED");
			} break;
			case ResourceType::ePipeline:
			{
				assert(false && "NOT IMPLEMENTED");
			} break;
			case ResourceType::ePipelineCache:
			{
				auto* handle = static_cast<VulkanPipelineCache*>(resource_handle);
				debug_utils->SetDebugName(handle->GetHandle(), name);
			} break;
			case ResourceType::eDescriptorPool:
			{
				assert(false && "NOT IMPLEMENTED");
			} break;
			case ResourceType::eDescriptorSet:
			{
				assert(false && "NOT IMPLEMENTED");
			} break;
			case ResourceType::eQueryPool:
			{
				auto* handle = static_cast<VulkanQueryPool*>(resource_handle);
				debug_utils->SetDebugName(handle->GetHandle(), name);
			} break;
			default:
				assert(false && "Unknown resource type");
				break;
			}
		}

		void VkGFXContext::EnableDeviceFeatures(PhysicalDevice& device)
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
				//else if(strcmp(extension_name, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME) == 0)
				//	device.RequestExtensionFeatures(vk::PhysicalDeviceIndirect)
			}
		}
	}
}