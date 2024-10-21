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
			auto& new_fence = fences.emplace_back(std::make_unique<VulkanFence>(*device, allocation_callbacks));
			return new_fence.get();
		}

		void VkGFXContext::DestroyFence(Fence* fence)
		{
			auto to_delete = std::remove_if(fences.begin(), fences.end(), [fence](const std::unique_ptr<VulkanFence>& f) { return f.get() == fence; });
			fences.erase(to_delete);
		}

		Semaphore* VkGFXContext::CreateSemaphore()
		{
			auto& new_semaphore = semaphores.emplace_back(std::make_unique<VulkanSemaphore>(*device, allocation_callbacks));
			return new_semaphore.get();
		}

		void VkGFXContext::DestroySemaphore(Semaphore* semaphore)
		{
			auto to_delete = std::remove_if(semaphores.begin(), semaphores.end(), [semaphore](const std::unique_ptr<VulkanSemaphore>& s) { return s.get() == semaphore; });
			semaphores.erase(to_delete);
		}

		Queue* VkGFXContext::RequestQueue(QueueTypeFlags queue_type_flags, uint32_t queue_index)
		{
			assert(false && "NOT IMPLEMENTED");
			return nullptr;
		}

		Swapchain* VkGFXContext::CreateSwapchain(const SwapChainCreateInfo& create_info)
		{
			assert(false && "NOT IMPLEMENTED");
			return nullptr;
		}

		void VkGFXContext::DestroySwapchain(Swapchain* swapchain)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		CommandPool* VkGFXContext::CreateCommandPool(Queue* queue)
		{
			assert(false && "NOT IMPLEMENTED");
			return nullptr;
		}

		void VkGFXContext::DestroyCommandPool(CommandPool* command_pool)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		Buffer* VkGFXContext::CreateBuffer(const BufferCreateInfo& create_info)
		{
			uint64_t minimal_alignment{ 1ull };
			vma::AllocationCreateInfo allocation_create_info{};
			vk::BufferCreateInfo buffer_create_info{};

			auto& physical = instance->GetBestDevice();
			auto& properties = physical.GetProperties();

			if (create_info.usage & BufferUsageFlag::eCopy)
			{
				buffer_create_info.usage |= vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
			}

			if (create_info.usage & BufferUsageFlag::eVertex)
			{
				buffer_create_info.usage |= vk::BufferUsageFlagBits::eVertexBuffer;
				minimal_alignment = std::max(minimal_alignment, 16ull);
			}

			if (create_info.usage & BufferUsageFlag::eIndex)
			{
				buffer_create_info.usage |= vk::BufferUsageFlagBits::eIndexBuffer;
				minimal_alignment = std::max(minimal_alignment, 4ull);
			}

			if (create_info.usage & BufferUsageFlag::eUniform)
			{
				buffer_create_info.usage |= vk::BufferUsageFlagBits::eUniformBuffer;
				minimal_alignment = std::lcm(properties.limits.minUniformBufferOffsetAlignment, properties.limits.nonCoherentAtomSize);
			}

			if (create_info.usage & BufferUsageFlag::eStorage)
			{
				buffer_create_info.usage |= vk::BufferUsageFlagBits::eStorageBuffer;
				minimal_alignment = std::max(minimal_alignment, properties.limits.minStorageBufferOffsetAlignment);
			}

			if (create_info.usage & BufferUsageFlag::eIndirect)
			{
				buffer_create_info.usage |= vk::BufferUsageFlagBits::eIndirectBuffer;
			}

			if (create_info.usage & BufferUsageFlag::eHasDeviceAddress)
			{
				buffer_create_info.usage |= vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;
			}

			if (create_info.usage & BufferUsageFlag::eGpuOnly)
				allocation_create_info.usage = vma::MemoryUsage::eGpuOnly;
			else if (create_info.usage & BufferUsageFlag::eCpuToGpu)
				allocation_create_info.usage = vma::MemoryUsage::eCpuToGpu;
			else
				allocation_create_info.usage = vma::MemoryUsage::eCpuOnly;

			buffer_create_info.size = CalculateAlignedSize(create_info.size, minimal_alignment) * create_info.count;

			buffer_create_info.sharingMode = vk::SharingMode::eConcurrent;

			auto family_count = device->GetQueueCount();

			std::array<uint32_t, 8u> queue_families{};
			std::iota(queue_families.begin(), queue_families.begin() + family_count, 0u);
			buffer_create_info.queueFamilyIndexCount = family_count;
			buffer_create_info.pQueueFamilyIndices = &queue_families[0];
			
			auto& new_buffer = buffers.emplace_back(std::make_unique<VulkanBuffer>(*allocator, buffer_create_info, allocation_create_info));
			return new_buffer.get();
		}

		void VkGFXContext::DestroyBuffer(Buffer* buffer)
		{
			auto to_delete = std::remove_if(buffers.begin(), buffers.end(), [buffer](const std::unique_ptr<VulkanBuffer>& b) { return b.get() == buffer; });
			buffers.erase(to_delete);
		}

		Texture* VkGFXContext::CreateTexture(const TextureCreateInfo& create_info)
		{
			assert(false && "NOT IMPLEMENTED");
			return nullptr;
		}

		void VkGFXContext::DestroyTexture(Texture* texture)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		Shader* VkGFXContext::CreateShader(const ShaderCreateInfo& create_info)
		{
			assert(false && "NOT IMPLEMENTED");
			return nullptr;
		}

		void VkGFXContext::DestroyShader(Shader* shader)
		{
			assert(false && "NOT IMPLEMENTED");
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
			vk::PipelineCacheCreateInfo vk_create_info{};
			vk_create_info.pInitialData = create_info.data.data();
			vk_create_info.initialDataSize = create_info.data.size();

			auto& new_pipeline_cache = pipeline_caches.emplace_back(std::make_unique<VulkanPipelineCache>(*device, allocation_callbacks, vk_create_info));
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
			vk::SamplerCreateInfo vk_create_info{};
			vk_create_info.minFilter = FilterToVk(create_info.min_filter);
			vk_create_info.magFilter = FilterToVk(create_info.mag_filter);
			vk_create_info.mipmapMode = MipmapModeToVk(create_info.mip_map_mode);
			vk_create_info.addressModeU = AddressModeToVk(create_info.address_u);
			vk_create_info.addressModeV = AddressModeToVk(create_info.address_v);
			vk_create_info.addressModeW = AddressModeToVk(create_info.address_w);
			vk_create_info.mipLodBias = create_info.mip_lod_bias;
			vk_create_info.minLod = create_info.min_lod;
			vk_create_info.maxLod = create_info.max_lod;
			vk_create_info.anisotropyEnable = create_info.max_anisotropy > 0.f;

			auto& physical = instance->GetBestDevice();
			auto& properties = physical.GetProperties();
			vk_create_info.maxAnisotropy = std::min(create_info.max_anisotropy, properties.limits.maxSamplerAnisotropy);

			vk_create_info.compareEnable = create_info.depth_compare != CompareOp::eNone;
			vk_create_info.compareOp = CompareOpToVk(create_info.depth_compare);

			auto& new_sampler = samplers.emplace_back(std::make_unique<VulkanSampler>(*device, allocation_callbacks, vk_create_info));
			return new_sampler.get();
		}

		void VkGFXContext::DestroySampler(Sampler* sampler)
		{
			auto to_delete = std::remove_if(samplers.begin(), samplers.end(), [sampler](const std::unique_ptr<VulkanSampler>& s) { return s.get() == sampler; });
			samplers.erase(to_delete);
		}

		QueryPool* VkGFXContext::CreateQueryPool()
		{
			return nullptr;
		}

		void VkGFXContext::DestroyQueryPool(QueryPool* query_pool)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VkGFXContext::WaitForFences(uint32_t fence_count, Fence** fences)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VkGFXContext::RequestTimestampFrequency(double* frequency)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VkGFXContext::RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VkGFXContext::SetResourceName(GFXResource* resource_handle, const char* name)
		{
			assert(false && "NOT IMPLEMENTED");
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