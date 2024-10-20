#include <RenderDevice.h>

#include <sstream>
#include <set>

#include <hash.hpp>
#include "VkCommon.h"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_to_string.hpp>

#define PROFILE_VULKAN
#include <engine/profiler/Profiler.h>

#include <engine/runtime/Window.h>

#include <base/PhysicalDevice.h>
#include <base/DebugUtils.h>
#include <base/Device.h>
#include <base/Allocator.h>

#include <volk.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			template<typename _HandleType, typename _CreateFunc>
			_HandleType VkCreateHandleAndCheckResult(_CreateFunc create_func)
			{
				_HandleType handle{ VK_NULL_HANDLE };
				vk::Result result = create_func(handle);
				if (result != vk::Result::eSuccess) 
				{
					log_error("Failed to create handle of type {} with result: {}", vk::to_string(handle.objectType), vk::to_string(result));
					return nullptr;
				}
				return handle;
			}

			vk::QueueFlags GetQueueFlags(EQueueTypeFlagBits flag)
			{
				vk::QueueFlags flags{};

				if (flag == EQueueTypeFlagBits::eGraphics)
					flags |= vk::QueueFlagBits::eGraphics;
				if (flag == EQueueTypeFlagBits::eCompute)
					flags |= vk::QueueFlagBits::eCompute;
				if (flag == EQueueTypeFlagBits::eTransfer)
					flags |= vk::QueueFlagBits::eTransfer;

				return flags;
			}

			void* vk_malloc(void* pUserData, u64 size, u64 alignment, VkSystemAllocationScope allocationScope)
			{
				void* ptr = malloc(size);
				memset(ptr, 0, size);
				return ptr;
			}

			void vk_free(void* pUserData, void* pMemory)
			{
				free(pMemory);
			}

			void* vk_realloc(void* pUserData, void* pOriginal, u64 size, u64 alignment, VkSystemAllocationScope allocationScope)
			{
				return realloc(pOriginal, size);
			}

			void vk_malloc_notification(void* pUserData, u64  size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
			{
			}

			void vk_free_notification(void* pUserData, u64 size, VkInternalAllocationType  allocationType, VkSystemAllocationScope allocationScope)
			{
			}

			u32 get_api_version(ERenderApi version)
			{
				switch (version)
				{
				case ERenderApi::eVulkan10:
					return VK_API_VERSION_1_0;
				case ERenderApi::eVulkan11:
					return VK_API_VERSION_1_1;
				case ERenderApi::eVulkan12:
					return VK_API_VERSION_1_2;
				case ERenderApi::eVulkan13:
					return VK_API_VERSION_1_3;
				}

				return 0u;
			}

			RenderDevice::~RenderDevice()
			{
				Destroy(pipeline_cache);
				Handle(*instance).destroySurfaceKHR(surface, allocation_callbacks);

				if (allocation_callbacks)
				{
					delete allocation_callbacks;
					allocation_callbacks = nullptr;
				}

#if !defined(BUILD_WITHOUT_DEBUGGIND) && defined(ENABLE_TRACY_PROFILING)
				for (auto& context : tracy_context)
					TracyVkDestroy(context);
				TracyVkDestroy(present_tracy_context);
#endif
			}

			void RenderDevice::AddDeviceExtension(const char* extension)
			{
				device_extensions.emplace_back(extension);
			}

			void RenderDevice::Create(const FRenderContextCreateInfo& context_ci)
			{
				// Register dispatcher
				PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
				VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

				// Create allocation callbacks
				//RegisterAllocationCallbacks();

				auto* window = context_ci.window;

				bool headless = window->GetWindowMode() == Window::Mode::Headless;

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
				instance_builder.AddRequiredExtensions(context_ci.requiredInstanceExtensions);

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

				surface = context_ci.window->CreateSurface(Handle(*instance), allocation_callbacks);
				if (!surface)
				{
					throw std::runtime_error("Failed to create window surface.");
				}

				// Creating vulkan device, specifying the swapchain extension always
				if (!headless || instance->IsEnabled(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
				{
					AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
					
					if (instance->IsEnabled(VK_KHR_DISPLAY_EXTENSION_NAME))
					{
						// Optional
						AddDeviceExtension(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME);
					}
				}

#ifdef VULKAN_ENABLE_PORTABILITY
				// VK_KHR_portability_subset must be enabled if present in the implementation (e.g on macOS/iOS with beta extensions enabled)
				// Optional
				AddDeviceExtension(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

				// Apply required device extensions
				device_extensions.insert(device_extensions.end(), context_ci.requiredDeviceExtensions.begin(), context_ci.requiredDeviceExtensions.end());

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
						AddDeviceExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
					}
				}
#endif

				// Enable vulkan features
				EnableDeviceFeatures(physical_device);

				// Enable vk 1.0 features
				auto& requested_features = physical_device.GetMutableRequestedFeatures();
				requested_features.samplerAnisotropy = true;
				requested_features.sampleRateShading = true;
				requested_features.fillModeNonSolid = true;
				requested_features.multiViewport = true;
				requested_features.geometryShader = true;
				requested_features.tessellationShader = true;
				requested_features.fragmentStoresAndAtomics = true;
				requested_features.logicOp = true;
				requested_features.imageCubeArray = true;
				requested_features.depthClamp = true;
				requested_features.independentBlend = true;
				requested_features.multiDrawIndirect = true;
				requested_features.shaderInt16 = true;
				requested_features.shaderStorageImageReadWithoutFormat = true;
				requested_features.shaderStorageImageWriteWithoutFormat = true;
				requested_features.shaderInt64 = true;

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

				vk::PipelineCacheCreateInfo pipeline_cache_ci{};
				pipeline_cache_ci.initialDataSize = utl::cast<u32>(context_ci.pipelineCache.size());
				pipeline_cache_ci.pInitialData = context_ci.pipelineCache.data();
				pipeline_cache = CreatePipelineCache(pipeline_cache_ci, "pipeline_cache");
			}

			vk::Buffer RenderDevice::CreateBuffer(Allocation& allocation, vk::BufferCreateInfo const& create_info, const std::string& debug_name) const
			{
				auto handle = allocator->CreateBuffer(allocation, create_info);
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			uint64_t RenderDevice::GetBufferDeviceAddress(const vk::Buffer& handle) const
			{
				return Handle(*device).getBufferAddressKHR(handle);
			}

			void RenderDevice::Destroy(Allocation& allocation, vk::Buffer& handle) const
			{
				allocator->DestroyBuffer(allocation, handle);
			}

			vk::Image RenderDevice::CreateImage(Allocation& allocation, vk::ImageCreateInfo const& create_info, const std::string& debug_name) const
			{
				auto handle = allocator->CreateImage(allocation, create_info);
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			void RenderDevice::Destroy(Allocation& allocation, vk::Image& handle) const
			{
				allocator->DestroyImage(allocation, handle);
			}

			vk::ImageView RenderDevice::CreateImageView(const vk::ImageViewCreateInfo& create_info, const std::string& debug_name) const
			{
				vk::ImageView handle = VkCreateHandleAndCheckResult<vk::ImageView>([&](auto& handle) { return Handle(*device).createImageView(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			void RenderDevice::Destroy(vk::ImageView& image_view) const
			{
				Handle(*device).destroyImageView(image_view, allocation_callbacks);
			}

			vk::SwapchainKHR RenderDevice::CreateSwapchain(const vk::SwapchainCreateInfoKHR& create_info, const std::string& debug_name) const
			{
				vk::SwapchainKHR handle = VkCreateHandleAndCheckResult<vk::SwapchainKHR>([&](auto& handle) { return Handle(*device).createSwapchainKHR(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			void RenderDevice::GetSwapchainImages(const vk::SwapchainKHR& handle, std::vector<vk::Image>& images, uint32_t& image_count, const std::string& debug_name)
			{
				images.resize(image_count);
				vk::Result result = Handle(*device).getSwapchainImagesKHR(handle, &image_count, images.data());
				if (result != vk::Result::eSuccess)
				{
					log_error("Failed to get swapchain images with reason: {}.", vk::to_string(result));
					return;
				}

				if (!debug_name.empty())
				{
					for (int32_t i = 0; i < image_count; ++i)
					{
						auto image_name = std::format("{}_{}", debug_name, i);
						debug_utils->SetDebugName(images[i], image_name.c_str());
					}
				}
			}

			void RenderDevice::Destroy(vk::SwapchainKHR& handle) const
			{
				Handle(*device).destroySwapchainKHR(handle, allocation_callbacks);
			}

			vk::PipelineCache RenderDevice::CreatePipelineCache(const vk::PipelineCacheCreateInfo& create_info, const std::string& debug_name) const
			{
				vk::PipelineCache handle = VkCreateHandleAndCheckResult<vk::PipelineCache>([&](auto& handle) { return Handle(*device).createPipelineCache(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			void RenderDevice::Destroy(vk::PipelineCache& handle) const
			{
				Handle(*device).destroyPipelineCache(handle, allocation_callbacks);
			}

			vk::Sampler RenderDevice::CreateSampler(const vk::SamplerCreateInfo& create_info, const std::string& debug_name) const
			{
				vk::Sampler handle = VkCreateHandleAndCheckResult<vk::Sampler>([&](auto& handle) { return Handle(*device).createSampler(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			void RenderDevice::Destroy(vk::Sampler& handle) const
			{
				Handle(*device).destroySampler(handle, allocation_callbacks);
			}

			vk::Semaphore RenderDevice::CreateSemaphore(const vk::SemaphoreCreateInfo& create_info, const std::string& debug_name) const
			{
				auto handle = VkCreateHandleAndCheckResult<vk::Semaphore>([&](auto& handle) { return Handle(*device).createSemaphore(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			void RenderDevice::Destroy(vk::Semaphore& handle) const
			{
				Handle(*device).destroySemaphore(handle, allocation_callbacks);
			}

			vk::Fence RenderDevice::CreateFence(const vk::FenceCreateInfo& create_info, const std::string& debug_name) const
			{
				auto handle = VkCreateHandleAndCheckResult<vk::Fence>([&](auto& handle) { return Handle(*device).createFence(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			vk::Result RenderDevice::WaitForFences(vk::Fence* handles, uint32_t fence_count, vk::Bool32 wait_for_all, uint64_t timeout) const
			{
				return Handle(*device).waitForFences(fence_count, handles, wait_for_all, timeout);
			}

			vk::Result RenderDevice::WaitForFences(std::span<vk::Fence> handles, vk::Bool32 wait_for_all, uint64_t timeout) const
			{
				return WaitForFences(handles.data(), handles.size(), wait_for_all, timeout);
			}

			vk::Result RenderDevice::ResetFences(vk::Fence* handles, uint32_t fence_count) const
			{
				return Handle(*device).resetFences(fence_count, handles);
			}

			vk::Result RenderDevice::ResetFences(std::span<vk::Fence> handles) const
			{
				return ResetFences(handles.data(), handles.size());
			}

			void RenderDevice::Destroy(vk::Fence& handle) const
			{
				Handle(*device).destroyFence(handle, allocation_callbacks);
			}

			vk::Pipeline RenderDevice::CreateGraphicsPipeline(const vk::GraphicsPipelineCreateInfo& create_info, const std::string& debug_name) const
			{
				vk::Pipeline handle{ VK_NULL_HANDLE };
				vk::Result result = Handle(*device).createGraphicsPipelines(pipeline_cache, 1u, &create_info, allocation_callbacks, &handle);
				if (result != vk::Result::eSuccess)
				{
					log_error("Failed to create handle of type {} with result: {}", vk::to_string(handle.objectType), vk::to_string(result));
					return nullptr;
				}

				debug_utils->SetDebugName(handle, debug_name);

				return handle;
			}

			vk::Pipeline RenderDevice::CreateComputePipeline(const vk::ComputePipelineCreateInfo& create_info, const std::string& debug_name) const
			{
				vk::Pipeline handle{ VK_NULL_HANDLE };
				vk::Result result = Handle(*device).createComputePipelines(pipeline_cache, 1u, &create_info, allocation_callbacks, &handle);
				if (result != vk::Result::eSuccess)
				{
					log_error("Failed to create handle of type {} with result: {}", vk::to_string(handle.objectType), vk::to_string(result));
					return nullptr;
				}

				debug_utils->SetDebugName(handle, debug_name);

				return handle;
			}

			void RenderDevice::Destroy(vk::Pipeline& handle) const
			{
				Handle(*device).destroyPipeline(handle, allocation_callbacks);
			}

			vk::CommandPool RenderDevice::CreateCommandPool(const vk::CommandPoolCreateInfo& create_info, const std::string& debug_name) const
			{
				auto handle = VkCreateHandleAndCheckResult<vk::CommandPool>([&](auto& handle) { return Handle(*device).createCommandPool(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			void RenderDevice::ResetCommandPool(vk::CommandPool& handle) const
			{
				Handle(*device).resetCommandPool(handle);
			}

			void RenderDevice::AllocateCommandBuffers(const vk::CommandBufferAllocateInfo& allocate_info, vk::CommandBuffer* command_buffers) const
			{
				Handle(*device).allocateCommandBuffers(&allocate_info, command_buffers);
			}

			void RenderDevice::FreeCommandBuffers(vk::CommandPool& command_pool, vk::CommandBuffer* command_buffers, uint32_t command_buffer_count)
			{
				Handle(*device).freeCommandBuffers(command_pool, command_buffer_count, command_buffers);
			}

			void RenderDevice::FreeCommandBuffers(vk::CommandPool& command_pool, std::span<vk::CommandBuffer> command_buffers)
			{
				FreeCommandBuffers(command_pool, command_buffers.data(), command_buffers.size());
			}

			void RenderDevice::Destroy(vk::CommandPool& handle) const
			{
				Handle(*device).destroyCommandPool(handle, allocation_callbacks);
			}

			vk::DescriptorSetLayout RenderDevice::CreateDescriptorSetLayout(const vk::DescriptorSetLayoutCreateInfo& create_info, const std::string& debug_name) const
			{
				auto handle = VkCreateHandleAndCheckResult<vk::DescriptorSetLayout>([&](auto& handle) { return Handle(*device).createDescriptorSetLayout(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			void RenderDevice::Destroy(vk::DescriptorSetLayout& handle) const
			{
				Handle(*device).destroyDescriptorSetLayout(handle, allocation_callbacks);
			}

			vk::DescriptorPool RenderDevice::CreateDescriptorPool(const vk::DescriptorPoolCreateInfo& create_info, const std::string& debug_name) const
			{
				auto handle = VkCreateHandleAndCheckResult<vk::DescriptorPool>([&](auto& handle) { return Handle(*device).createDescriptorPool(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			void RenderDevice::ResetDescriptorPool(vk::DescriptorPool& handle, vk::DescriptorPoolResetFlags flags) const
			{
				Handle(*device).resetDescriptorPool(handle, flags);
			}

			void RenderDevice::Destroy(vk::DescriptorPool& handle) const
			{
				Handle(*device).destroyDescriptorPool(handle, allocation_callbacks);
			}

			void RenderDevice::AllocateDescriptorSet(const vk::DescriptorSetAllocateInfo& allocate_info, vk::DescriptorSet* descriptor_sets, const std::string& debug_name) const
			{
				vk::Result result = Handle(*device).allocateDescriptorSets(&allocate_info, descriptor_sets);
				if (result != vk::Result::eSuccess)
				{
					log_error("Failed to allocate descriptor set with result: {}", vk::to_string(result));
					return;
				}

				for (int32_t i = 0; i < allocate_info.descriptorSetCount; ++i)
				{
					auto& handle = descriptor_sets[i];
					debug_utils->SetDebugName(handle, std::format("{}_{}", debug_name, i));
				}
			}

			void RenderDevice::FreeDescriptorSets(vk::DescriptorPool& pool, vk::DescriptorSet* descriptor_sets, uint32_t descriptor_set_count) const
			{
				Handle(*device).freeDescriptorSets(pool, descriptor_set_count, descriptor_sets);
			}

			void RenderDevice::FreeDescriptorSets(vk::DescriptorPool& pool, std::span<vk::DescriptorSet> descriptor_sets) const
			{
				FreeDescriptorSets(pool, descriptor_sets.data(), descriptor_sets.size());
			}

			vk::ShaderModule RenderDevice::CreateShaderModule(const vk::ShaderModuleCreateInfo& create_info, const std::string& debug_name) const
			{
				auto handle = VkCreateHandleAndCheckResult<vk::ShaderModule>([&](auto& handle) { return Handle(*device).createShaderModule(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			void RenderDevice::Destroy(vk::ShaderModule& handle)
			{
				Handle(*device).destroyShaderModule(handle);
			}

			vk::PipelineLayout RenderDevice::CreatePipelineLayout(const vk::PipelineLayoutCreateInfo& create_info, const std::string& debug_name) const
			{
				auto handle = VkCreateHandleAndCheckResult<vk::PipelineLayout>([&](auto& handle) { return Handle(*device).createPipelineLayout(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			void RenderDevice::Destroy(vk::PipelineLayout& handle) const
			{
				Handle(*device).destroyPipelineLayout(handle);
			}

			vk::QueryPool RenderDevice::CreateQueryPool(const vk::QueryPoolCreateInfo& create_info, const std::string& debug_name) const
			{
				auto handle = VkCreateHandleAndCheckResult<vk::QueryPool>([&](auto& handle) { return Handle(*device).createQueryPool(&create_info, allocation_callbacks, &handle); });
				debug_utils->SetDebugName(handle, debug_name);
				return handle;
			}

			vk::Result RenderDevice::GetQueryResult(vk::QueryPool& handle, uint32_t first_query, uint32_t query_count, vk::DeviceSize stride, vk::QueryResultFlags flags) const
			{
				//auto result = Handle(*device).getQueryPoolResult(handle, first_query, query_count, stride, flags);
				return vk::Result::eIncomplete;
			}

			vk::Result RenderDevice::GetQueryResults(vk::QueryPool& handle, uint32_t first_query, uint32_t query_count, vk::DeviceSize data_size, void* data_ptr, vk::DeviceSize stride, vk::QueryResultFlags flags) const
			{
				return Handle(*device).getQueryPoolResults(handle, first_query, query_count, data_size, data_ptr, stride, flags);
			}

			void RenderDevice::ResetQueryPool(vk::QueryPool& handle, uint32_t first_query, uint32_t query_count) const
			{
				Handle(*device).resetQueryPool(handle, first_query, query_count);
			}

			void RenderDevice::Destroy(vk::QueryPool& handle) const
			{
				Handle(*device).destroyQueryPool(handle);
			}

			void RenderDevice::Wait()
			{
				Handle(*device).waitIdle();
			}

			void RenderDevice::GetOccupedDeviceMemory(vk::MemoryHeapFlags memory_flags, u64& total_memory, u64& used_memory)
			{
				vk::PhysicalDeviceMemoryBudgetPropertiesEXT memory_budget_props{};
				vk::PhysicalDeviceMemoryProperties2 memory_props{};
				memory_props.pNext = &memory_budget_props;

				auto& physical_device = instance->GetBestDevice();
				Handle(physical_device).getMemoryProperties2(&memory_props);

				total_memory = used_memory = 0ull;

				auto heaps_count = static_cast<u64>(memory_props.memoryProperties.memoryHeapCount);
				for (u64 i = 0ull; i < heaps_count; ++i)
				{
					auto heapFlags = memory_props.memoryProperties.memoryHeaps[i].flags;
					if (heapFlags & memory_flags)
					{
						total_memory += memory_budget_props.heapBudget[i];
						used_memory += memory_budget_props.heapUsage[i];
					}
				}
			}

			void RenderDevice::SubmitCommandBuffer(vk::CommandBuffer& command_buffer)
			{
				vk::SubmitInfo submitInfo{};
				submitInfo.commandBufferCount = 1u;
				submitInfo.pCommandBuffers = &command_buffer;

				vk::FenceCreateInfo fence_ci{};
				vk::Fence fence = CreateFence(fence_ci);

				Handle(*device).resetFences(1, &fence);

				// TODO: COMMAND BUFFERS
				auto& queue = device->GetQueueByFlags(vk::QueueFlagBits::eGraphics, 0);
				queue.Submit(submitInfo, fence);
				Handle(*device).waitForFences(1, &fence, VK_TRUE, (std::numeric_limits<u64>::max)());
				Handle(*device).destroyFence(fence, allocation_callbacks);
			}

			void RenderDevice::InitProfiler(vk::CommandBuffer& command_pool)
			{
#if !defined(BUILD_WITHOUT_DEBUGGIND) && defined(ENABLE_TRACY_PROFILING)
				FVkCommandBufferCreateInfo command_buffer_ci{};
				command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
				command_buffer_ci.count = 1u;

				FVkCommandBuffer tracy_cmd_buffer{};
				CreateCommandBuffer(command_buffer_ci, command_pool, tracy_cmd_buffer);

				auto& physical_device = GetPhysicalDevice();

				auto& graphics = m_logicalDevice.queues[EQueueTypeFlagBits::eGraphics];
				auto& present = m_logicalDevice.queues[EQueueTypeFlagBits::ePresent];

				tracy_context.resize(swapchain.framesInFlight);
				for (u32 idx = 0u; idx < swapchain.framesInFlight; ++idx)
					tracy_context[idx] = TracyVkContext(Handle(physical_device), Handle(*device), graphics, tracy_cmd_buffer.handles.back());

				present_tracy_context = TracyVkContext(Handle(physical_device), Handle(*device), present, tracy_cmd_buffer.handles.back());

				graphics.waitIdle();
				DestroyCommandBuffer(command_pool, tracy_cmd_buffer);
#endif
			}

			vk::Format RenderDevice::FindSupportedFormat(const utl::vector<vk::Format>& candidates, bool depth_only)
			{
				auto& phusical_device = instance->GetBestDevice();

				vk::Format depth_format{ vk::Format::eUndefined };
				for (const auto& format : candidates)
				{
					if (depth_only && !is_depth_only_format(format))
						continue;

					vk::FormatProperties format_properties;
					Handle(phusical_device).getFormatProperties(format, &format_properties);

					// Format must support depth stencil attachment for optimal tiling
					if (format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
					{
						depth_format = format;
						break;
					}
				}

				return depth_format;
			}

			const vk::DynamicLoader& RenderDevice::GetDynamicLoader() const
			{
				return dl;
			}

			const vk::AllocationCallbacks* RenderDevice::GetAllocationCallbacks() const
			{
				return allocation_callbacks;
			}

			const vk::SurfaceKHR& RenderDevice::GetSurface() const
			{
				return surface;
			}

			const vk::SampleCountFlagBits RenderDevice::GetMultisamplingCount() const
			{
				return required_sample_count;
			}

			Instance& RenderDevice::GetInstance() const
			{
				return *instance;
			}

			PhysicalDevice& RenderDevice::GetPhysicalDevice() const
			{
				return instance->GetBestDevice();
			}

			Device& RenderDevice::GetLogicalDevice() const
			{
				return *device;
			}

			DebugUtils& RenderDevice::GetDebugUtils() const
			{
				return *debug_utils;
			}

			Allocator& RenderDevice::GetAllocator() const
			{
				return *allocator;
			}

			vma::Allocator& RenderDevice::GetMemoryAllocator()
			{
				return Handle(*allocator);
			}

			const vk::PipelineCache& RenderDevice::GetPipelineCache() const
			{
				return pipeline_cache;
			}

			utl::vector<char> RenderDevice::GetPipelineCacheData() const
			{
				utl::vector<char> pipeline_cache_data{};

				u64 data_size{ 0ull };
				void* data_ptr{ nullptr };
				vk::Result res = Handle(*device).getPipelineCacheData(pipeline_cache, &data_size, data_ptr);

				if (res == vk::Result::eSuccess)
				{
					pipeline_cache_data.resize(data_size);
					util::Copy(pipeline_cache_data.data(), data_ptr, data_size);
				}

				return pipeline_cache_data;
			}

			const std::vector<vk::SurfaceFormatKHR>& RenderDevice::GetSurfacePriorityList() const
			{
				return surface_priority_list;
			}

			void RenderDevice::RegisterAllocationCallbacks()
			{
				allocation_callbacks = new vk::AllocationCallbacks();
				memset(allocation_callbacks, 0, sizeof(vk::AllocationCallbacks));

				allocation_callbacks->pfnAllocation = (PFN_vkAllocationFunction)(&vk_malloc);
				allocation_callbacks->pfnReallocation = (PFN_vkReallocationFunction)(&vk_realloc);
				allocation_callbacks->pfnFree = (PFN_vkFreeFunction)(&vk_free);
				allocation_callbacks->pfnInternalAllocation = (PFN_vkInternalAllocationNotification)(&vk_malloc_notification);
				allocation_callbacks->pfnInternalFree = (PFN_vkInternalFreeNotification)(&vk_free_notification);
			}

			void RenderDevice::EnableDeviceFeatures(PhysicalDevice& device)
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
}