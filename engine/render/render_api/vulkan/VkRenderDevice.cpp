#include "VkRenderDevice.h"

#include <sstream>
#include <set>

#include <hash.hpp>
#include "VkCommon.h"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_to_string.hpp>

#define PROFILE_VULKAN
#include <engine/profiler/Profiler.h>

#include <engine/runtime/Window.h>

#include <engine/render/render_api/vulkan/PhysicalDevice.h>
#include <engine/render/render_api/vulkan/DebugUtils.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/Allocator.h>

#include <volk.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
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

			VkRenderDevice::~VkRenderDevice()
			{
				CleanupSwapchain();
				device->DestroyObject(swapchain.handle);
				device->DestroyObject(pipeline_cache);
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

			void VkRenderDevice::AddDeviceExtension(const char* extension)
			{
				device_extensions.emplace_back(extension);
			}

			void VkRenderDevice::AddInstanceExtension(const char* extension)
			{
				instance_extensions.emplace_back(extension);
			}

			void VkRenderDevice::AddLayerSetting(vk::LayerSettingEXT const& layerSetting)
			{
				layer_settings.emplace_back(layerSetting);
			}

			void VkRenderDevice::Create(const FRenderContextCreateInfo& context_ci)
			{
				// Register dispatcher
				PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
				VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

				// Create allocation callbacks
				//RegisterAllocationCallbacks();

				auto* window = context_ci.window;

				bool headless = window->GetWindowMode() == Window::Mode::Headless;

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
					instance_extensions.insert(instance_extensions.end(), required_surface_extensions.begin(), required_surface_extensions.end());
				}

				// Apply extensions requested by user
				instance_extensions.insert(instance_extensions.end(), context_ci.requiredInstanceExtensions.begin(), context_ci.requiredInstanceExtensions.end());

#ifdef VULKAN_DEBUG
				{
					std::vector<vk::ExtensionProperties> available_instance_extensions = vk::enumerateInstanceExtensionProperties();
					auto debugExtensionIt = std::find_if(available_instance_extensions.begin(), available_instance_extensions.end(),
						[](vk::ExtensionProperties const& ep) { return strcmp(ep.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0; });
					if (debugExtensionIt != available_instance_extensions.end())
					{
						log_info("Vulkan debug utils enabled ({})", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
						AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
					}
				}
#endif

				InstanceCreateInfo instance_ci{ "App", instance_extensions, validation_layers, layer_settings, headless, get_api_version(context_ci.apiVersion) };
				instance = std::make_unique<Instance>(instance_ci);

				surface = context_ci.window->CreateSurface(Handle(*instance), allocation_callbacks);
				if (!surface)
				{
					throw std::runtime_error("Failed to create window surface.");
				}

				// Creating vulkan device, specifying the swapchain extension always
				if (!headless || instance->IsEnabled(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
				{
					AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

					if (std::find(instance_extensions.begin(), instance_extensions.end(), VK_KHR_DISPLAY_EXTENSION_NAME) != instance_extensions.end())
					{
						AddDeviceExtension(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME);
					}
				}

#ifdef VULKAN_ENABLE_PORTABILITY
				// VK_KHR_portability_subset must be enabled if present in the implementation (e.g on macOS/iOS with beta extensions enabled)
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

				device = std::make_unique<Device>(physical_device, allocation_callbacks, surface, device_extensions);

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

				check_api_result(CreatePipelineCache(context_ci));
				auto& extent = context_ci.window->GetExtent();
				check_api_result(CreateSwapchain(extent.width, extent.height));
			}

			FRenderApiErrorInfo VkRenderDevice::CreateCommandPool(const FVkCommandPoolCreateInfo& command_pool_ci, FVkCommandPool& command_pool)
			{
				const bool is_compute_queue = (command_pool_ci.mask & EQueueTypeFlagBits::eCompute) != 0;
				const bool is_transfer_queue = (command_pool_ci.mask & EQueueTypeFlagBits::eTransfer) != 0;

				vk::CommandPoolCreateInfo pool_ci{};
				pool_ci.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

				if (command_pool_ci.mask & EQueueTypeFlagBits::ePresent)
				{
					auto& found = device->GetQueueByPresent(0);
					pool_ci.queueFamilyIndex = found.GetFamilyIndex();
				}
				else if (command_pool_ci.mask & EQueueTypeFlagBits::eGraphics)
				{
					pool_ci.queueFamilyIndex = device->GetQueueFamilyIndex(vk::QueueFlagBits::eGraphics);
				}
				else if (command_pool_ci.mask & EQueueTypeFlagBits::eCompute)
				{
					pool_ci.queueFamilyIndex = device->GetQueueFamilyIndex(vk::QueueFlagBits::eCompute);
				}
				else if (command_pool_ci.mask & EQueueTypeFlagBits::eTransfer)
				{
					pool_ci.queueFamilyIndex = device->GetQueueFamilyIndex(vk::QueueFlagBits::eTransfer);
				}

				vk::Result res = Handle(*device).createCommandPool(&pool_ci, allocation_callbacks, &command_pool.handle);
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::stringstream ss{};
				ss << command_pool_ci.threadId;
				debug_utils->SetDebugName(command_pool.handle, std::format("thread_id {}", ss.str()));
#endif

				return make_api_error(command_pool.handle.objectType, res, "Failed to Create");
			}

			void VkRenderDevice::DestroyCommandPool(FVkCommandPool& command_pool)
			{
				Handle(*device).destroyCommandPool(command_pool.handle, allocation_callbacks);
			}

			FRenderApiErrorInfo VkRenderDevice::CreateCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, const FVkCommandPool& command_pool, FVkCommandBuffer& command_buffer)
			{
				vk::CommandBufferAllocateInfo command_buffer_ai{};
				command_buffer_ai.commandPool = command_pool.handle;
				command_buffer_ai.level = command_buffer_ci.level;
				command_buffer_ai.commandBufferCount = command_buffer_ci.count;

				command_buffer.handles.resize(command_buffer_ci.count);

				vk::Result res = Handle(*device).allocateCommandBuffers(&command_buffer_ai, command_buffer.handles.data());
#ifndef BUILD_WITHOUT_DEBUGGIND
				for (i32 idx = 0; idx < command_buffer.handles.size(); ++idx)
					debug_utils->SetDebugName(command_buffer.get(), std::format("{} #{}", command_buffer_ci.name, idx));
#endif

				command_buffer.mask = command_buffer_ci.mask;

				return make_api_error(vk::ObjectType::eCommandBuffer, res, "Failed to allocate");
			}

			void VkRenderDevice::DestroyCommandBuffer(const FVkCommandPool& command_pool, FVkCommandBuffer& command_buffer)
			{
				Handle(*device).freeCommandBuffers(command_pool.handle, command_buffer.handles);
			}

			void VkRenderDevice::BeginCommandBuffer(FVkCommandBuffer& command_buffer, vk::CommandBufferUsageFlags usage, u32 image_index)
			{
				if (command_buffer.isRunning)
					return;

				command_buffer.commandBufferIndex = image_index;

				vk::CommandBufferBeginInfo begin_info{};
				begin_info.flags = usage;

				auto& current_command_buffer_handle = command_buffer.get();
				current_command_buffer_handle.begin(begin_info);
				command_buffer.isRunning = true;
			}

			void VkRenderDevice::EndCommandBuffer(FVkCommandBuffer& command_buffer)
			{
				if (!command_buffer.isRunning)
					return;

				auto& current_command_buffer_handle = command_buffer.get();
				current_command_buffer_handle.end();

				command_buffer.isRunning = false;
			}

			FRenderApiErrorInfo VkRenderDevice::CreateImage(const FVkImageCreateInfo& image_ci, FVkImage& image)
			{
				vma::AllocationCreateInfo alloc_ci{};
				alloc_ci.usage = image_ci.memoryUsage;

				vk::Result res = Handle(*allocator).createImage(&image_ci.ci, &alloc_ci, &image.handle, &image.memory, nullptr);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(image.handle, image_ci.name);
#endif
				return make_api_error(image.handle.objectType, res, "Failed to Create");
			}

			void VkRenderDevice::DestroyImage(FVkImage& image)
			{
				Handle(*allocator).destroyImage(image.handle, image.memory);
			}

			FRenderApiErrorInfo VkRenderDevice::CreateImageView(const FVkImageViewCreateInfo& image_view_ci, vk::ImageView& image_view)
			{
				vk::Result res = Handle(*device).createImageView(&image_view_ci.ci, allocation_callbacks, &image_view);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(image_view, image_view_ci.name);
#endif
				return make_api_error(image_view.objectType, res, "Failed to Create");
			}

			void VkRenderDevice::DestroyImageView(vk::ImageView& image_view)
			{
				Handle(*device).destroyImageView(image_view, allocation_callbacks);
			}

			FRenderApiErrorInfo VkRenderDevice::CreateSampler(const FVkSamplerCreateInfo& sampler_ci, vk::Sampler& sampler)
			{
				auto& physical_device = instance->GetBestDevice();

				vk::SamplerCreateInfo ci{};
				ci.magFilter = sampler_ci.magFilter;
				ci.minFilter = sampler_ci.minFilter;
				ci.addressModeU = sampler_ci.addressMode;
				ci.addressModeV = sampler_ci.addressMode;
				ci.addressModeW = sampler_ci.addressMode;
				ci.borderColor = vk::BorderColor::eIntOpaqueWhite;
				ci.unnormalizedCoordinates = VK_FALSE;

				// Anisotropy filtering
				ci.anisotropyEnable = sampler_ci.enableAnisotropy;
				ci.maxAnisotropy = physical_device.GetProperties().limits.maxSamplerAnisotropy;

				// Depth
				ci.compareEnable = sampler_ci.enableCompareOp;
				ci.compareOp = vk::CompareOp::eLess;

				ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
				ci.mipLodBias = 0.f;
				ci.minLod = 0.f;
				ci.maxLod = utl::cast<f32>(sampler_ci.mipLevels);

				vk::Result res = Handle(*device).createSampler(&ci, allocation_callbacks, &sampler);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(sampler, sampler_ci.name);
#endif
				return make_api_error(sampler.objectType, res, "Failed to Create");
			}

			void VkRenderDevice::DestroySampler(vk::Sampler& sampler)
			{
				Handle(*device).destroySampler(sampler, allocation_callbacks);
			}

			FRenderApiErrorInfo VkRenderDevice::CreateDescriptorSetLayout(const FVkDescriptorSetLayoutCreateInfo& layout_ci, vk::DescriptorSetLayout& descriptor_set_layout)
			{
				vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_ci{};
				descriptor_set_layout_ci.bindingCount = utl::cast<u32>(layout_ci.layoutBindings.size());
				descriptor_set_layout_ci.pBindings = layout_ci.layoutBindings.data();
				descriptor_set_layout_ci.flags = layout_ci.flags;
				descriptor_set_layout_ci.pNext = layout_ci.next;

				vk::Result res = Handle(*device).createDescriptorSetLayout(&descriptor_set_layout_ci, allocation_callbacks, &descriptor_set_layout);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(descriptor_set_layout, layout_ci.name);
#endif
				return make_api_error(descriptor_set_layout.objectType, res, "Failed to Create");
			}

			void VkRenderDevice::DestroyDescriptorSetLayout(vk::DescriptorSetLayout& descriptor_set_layout)
			{
				Handle(*device).destroyDescriptorSetLayout(descriptor_set_layout, allocation_callbacks);
			}

			FRenderApiErrorInfo VkRenderDevice::CreateDescriptorPool(const FVkDescriptorPoolCreateInfo& pool_ci, FVkDescriptorPool& descriptor_pool)
			{
				utl::vector<vk::DescriptorPoolSize> pool_sizes{ pool_ci.poolSizes };

				// Calculate total descriptor set count in descriptor pool and update descriptor sizes
				u32 maxSets{ 1u };
				for (auto& pool_size : pool_sizes)
				{
					pool_size.descriptorCount *= pool_ci.usageCount;
					maxSets += pool_size.descriptorCount;
				}

				vk::DescriptorPoolCreateInfo descriptor_pool_ci{};
				descriptor_pool_ci.poolSizeCount = utl::cast<u32>(pool_sizes.size());
				descriptor_pool_ci.pPoolSizes = pool_sizes.data();
				descriptor_pool_ci.maxSets = maxSets;
				descriptor_pool_ci.flags = pool_ci.flags;

				vk::Result res = Handle(*device).createDescriptorPool(&descriptor_pool_ci, allocation_callbacks, &descriptor_pool.handle);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(descriptor_pool.handle, pool_ci.name);
#endif

				return make_api_error(descriptor_pool.handle.objectType, res, "Failed to Create");
			}

			FRenderApiErrorInfo VkRenderDevice::GrowDescriptorPool(const FVkDescriptorPoolCreateInfo& pool_ci, FVkDescriptorPool& descriptor_pool)
			{
				DestroyDescriptorPool(descriptor_pool);
				return CreateDescriptorPool(pool_ci, descriptor_pool);
			}

			void VkRenderDevice::ResetDescriptorPool(FVkDescriptorPool& descriptor_pool)
			{
				Handle(*device).resetDescriptorPool(descriptor_pool.handle);
			}

			void VkRenderDevice::DestroyDescriptorPool(FVkDescriptorPool& descriptor_pool)
			{
				Handle(*device).destroyDescriptorPool(descriptor_pool.handle, allocation_callbacks);
			}

			FRenderApiErrorInfo VkRenderDevice::CreateDescriptorSet(const FVkDescriptorSetCreateInfo& set_ci, FVkDescriptorSet& descriptor_set)
			{
				utl::vector<vk::DescriptorSetLayout> per_frame_layout(set_ci.descriptorCount, set_ci.layout);

				vk::DescriptorSetAllocateInfo descriptor_ai{};
				descriptor_ai.descriptorPool = set_ci.pool;
				descriptor_ai.descriptorSetCount = utl::cast<u32>(per_frame_layout.size());
				descriptor_ai.pSetLayouts = per_frame_layout.data();
				descriptor_ai.pNext = set_ci.next;

				descriptor_set.handles.resize(set_ci.descriptorCount);
				vk::Result res = Handle(*device).allocateDescriptorSets(&descriptor_ai, descriptor_set.handles.data());

#ifndef BUILD_WITHOUT_DEBUGGIND
				for (u64 idx = 0ull; idx < descriptor_set.handles.size(); ++idx)
					debug_utils->SetDebugName(descriptor_set.handles[idx], std::format("{} #{}", set_ci.name, idx));
#endif

				return make_api_error(vk::ObjectType::eDescriptorSet, res, "Failed to allocate");
			}

			void VkRenderDevice::DestroyDescriptorSet(const FVkDescriptorPool& descriptor_pool, FVkDescriptorSet& descriptor_set)
			{
				Handle(*device).freeDescriptorSets(descriptor_pool.handle, descriptor_set.handles);
			}

			FRenderApiErrorInfo VkRenderDevice::CreateShaderModule(const FVkShaderModuleCreateInfo& shader_module_ci, vk::ShaderModule& shader_module)
			{
				vk::ShaderModuleCreateInfo module_ci{};
				module_ci.codeSize = shader_module_ci.shaderCode->size() * sizeof(u32);
				module_ci.pCode = shader_module_ci.shaderCode->data();

				vk::Result res = Handle(*device).createShaderModule(&module_ci, allocation_callbacks, &shader_module);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(shader_module, shader_module_ci.name);
#endif
				return make_api_error(shader_module.objectType, res, "Failed to Create");
			}

			void VkRenderDevice::DestroyShaderModule(vk::ShaderModule& shader_module)
			{
				Handle(*device).destroyShaderModule(shader_module, allocation_callbacks);
			}

			FRenderApiErrorInfo VkRenderDevice::CreatePipelineLayout(const FVkPipelineLayoutCreateInfo& pipeline_layout_ci, vk::PipelineLayout& pipeline_layout)
			{
				vk::PipelineLayoutCreateInfo layout_ci{};
				layout_ci.setLayoutCount = utl::cast<u32>(pipeline_layout_ci.descriptorSetLayouts.size());
				layout_ci.pSetLayouts = pipeline_layout_ci.descriptorSetLayouts.data();
				layout_ci.pushConstantRangeCount = utl::cast<u32>(pipeline_layout_ci.pushConstantRanges.size());
				layout_ci.pPushConstantRanges = pipeline_layout_ci.pushConstantRanges.data();
				vk::Result res = Handle(*device).createPipelineLayout(&layout_ci, allocation_callbacks, &pipeline_layout);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(pipeline_layout, pipeline_layout_ci.name);
#endif
				return make_api_error(pipeline_layout.objectType, res, "Failed to Create");
			}

			void VkRenderDevice::DestroyPipelineLayout(vk::PipelineLayout& pipeline_layout)
			{
				Handle(*device).destroyPipelineLayout(pipeline_layout, allocation_callbacks);
			}

			FRenderApiErrorInfo VkRenderDevice::CreatePipeline(const FVkGraphicsPipelineCreateInfo& pipeline_ci, vk::Pipeline& pipeline)
			{
				vk::PipelineVertexInputStateCreateInfo vertex_input_state{};
				vertex_input_state.vertexAttributeDescriptionCount = utl::cast<u32>(pipeline_ci.vertexAttributeDescriptions.size());
				vertex_input_state.pVertexAttributeDescriptions = pipeline_ci.vertexAttributeDescriptions.data();
				vertex_input_state.vertexBindingDescriptionCount = utl::cast<u32>(pipeline_ci.vertexBindingDescriptions.size());
				vertex_input_state.pVertexBindingDescriptions = pipeline_ci.vertexBindingDescriptions.data();

				utl::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment_states{};
				for (u32 idx = 0u; idx < pipeline_ci.colorAttachmentFormats.size(); ++idx)
				{
					auto& state = color_blend_attachment_states.emplace_back();

					// TODO: generate write mask from format type
					state.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
					state.blendEnable = pipeline_ci.enableAlphaBlend; // TODO: add color blending support

					state.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
					state.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
					state.colorBlendOp = vk::BlendOp::eAdd;
					state.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;//vk::BlendFactor::eOne;
					state.dstAlphaBlendFactor = vk::BlendFactor::eZero;
					state.alphaBlendOp = vk::BlendOp::eAdd;
				}

				vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
				input_assembly.topology = pipeline_ci.primitiveTopology;
				input_assembly.primitiveRestartEnable = false;

				vk::PipelineRasterizationStateCreateInfo rasterization_state{};
				rasterization_state.depthClampEnable = pipeline_ci.colorAttachmentFormats.empty() && pipeline_ci.depthAttachmentFormat != vk::Format::eUndefined;
				rasterization_state.rasterizerDiscardEnable = false;
				rasterization_state.polygonMode = vk::PolygonMode::eFill;
				rasterization_state.lineWidth = 1.0f;
				rasterization_state.cullMode = pipeline_ci.cullMode;
				rasterization_state.frontFace = pipeline_ci.frontFace;
				rasterization_state.depthBiasEnable = VK_FALSE;

				vk::PipelineMultisampleStateCreateInfo multisample_state{};
				multisample_state.rasterizationSamples = pipeline_ci.sampleCount;

				vk::PipelineColorBlendStateCreateInfo color_blending_state{};
				color_blending_state.logicOpEnable = false;
				color_blending_state.logicOp = vk::LogicOp::eCopy;
				color_blending_state.attachmentCount = utl::cast<u32>(color_blend_attachment_states.size());
				color_blending_state.pAttachments = color_blend_attachment_states.data();

				auto enable_depth = pipeline_ci.depthAttachmentFormat != vk::Format::eUndefined;
				vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{};
				depth_stencil_state.depthTestEnable = enable_depth;
				depth_stencil_state.depthWriteEnable = enable_depth; // TODO: depth write should be set by texture state

				depth_stencil_state.depthCompareOp = pipeline_ci.depthCompareOp;
				depth_stencil_state.back.compareOp = vk::CompareOp::eAlways;
				depth_stencil_state.depthBoundsTestEnable = false;
				depth_stencil_state.stencilTestEnable = false; // TODO: implement stencil test

				utl::vector<vk::DynamicState> dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::ePrimitiveTopology };

				vk::PipelineDynamicStateCreateInfo dynamic_state{};
				dynamic_state.pDynamicStates = dynamic_states.data();
				dynamic_state.dynamicStateCount = utl::cast<u32>(dynamic_states.size());

				vk::PipelineViewportStateCreateInfo viewport_state{};
				viewport_state.viewportCount = 1u;
				viewport_state.scissorCount = 1u;

				vk::PipelineTessellationStateCreateInfo tessellation_state{};
				tessellation_state.patchControlPoints = pipeline_ci.patchControlPoints;

				auto enableTessellation = pipeline_ci.primitiveTopology == vk::PrimitiveTopology::ePatchList;

				// VK_KHR_dynamic_rendering
				vk::PipelineRenderingCreateInfoKHR rendering_ci{};
				rendering_ci.colorAttachmentCount = utl::cast<u32>(pipeline_ci.colorAttachmentFormats.size());
				rendering_ci.pColorAttachmentFormats = pipeline_ci.colorAttachmentFormats.data();
				if (pipeline_ci.depthAttachmentFormat != vk::Format::eUndefined)
				{
					rendering_ci.depthAttachmentFormat = pipeline_ci.depthAttachmentFormat;
					if (pipeline_ci.stencilAttachmentFormat != pipeline_ci.depthAttachmentFormat && pipeline_ci.stencilAttachmentFormat != vk::Format::eUndefined)
						rendering_ci.stencilAttachmentFormat = pipeline_ci.stencilAttachmentFormat;
					else if (!is_depth_only_format(pipeline_ci.depthAttachmentFormat))
						rendering_ci.stencilAttachmentFormat = pipeline_ci.depthAttachmentFormat;
				}

				vk::GraphicsPipelineCreateInfo graphics_pipeline_ci{};
				graphics_pipeline_ci.stageCount = utl::cast<u32>(pipeline_ci.shaderStages.size());
				graphics_pipeline_ci.pStages = pipeline_ci.shaderStages.data();
				graphics_pipeline_ci.pVertexInputState = &vertex_input_state;
				graphics_pipeline_ci.pInputAssemblyState = &input_assembly;
				graphics_pipeline_ci.pViewportState = &viewport_state;
				graphics_pipeline_ci.pRasterizationState = &rasterization_state;
				graphics_pipeline_ci.pMultisampleState = &multisample_state;
				graphics_pipeline_ci.pColorBlendState = &color_blending_state;
				graphics_pipeline_ci.pDepthStencilState = &depth_stencil_state;
				graphics_pipeline_ci.layout = pipeline_ci.layout;
				graphics_pipeline_ci.renderPass = nullptr;
				graphics_pipeline_ci.subpass = 0;
				graphics_pipeline_ci.basePipelineHandle = nullptr;
				graphics_pipeline_ci.pDynamicState = &dynamic_state;
				graphics_pipeline_ci.pTessellationState = (enableTessellation ? &tessellation_state : nullptr);
				graphics_pipeline_ci.pNext = &rendering_ci;

				vk::Result res = Handle(*device).createGraphicsPipelines(pipeline_cache, 1u, &graphics_pipeline_ci, allocation_callbacks, &pipeline);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(pipeline, pipeline_ci.name);
#endif
				return make_api_error(pipeline.objectType, res, "Failed to Create");
			}

			FRenderApiErrorInfo VkRenderDevice::CreatePipeline(const FVkComputePipelineCreateInfo& pipeline_ci, vk::Pipeline& pipeline)
			{
				vk::ComputePipelineCreateInfo compute_pipeline_ci{};
				compute_pipeline_ci.stage = pipeline_ci.shaderStage;
				compute_pipeline_ci.layout = pipeline_ci.layout;
				compute_pipeline_ci.basePipelineHandle = nullptr;
				compute_pipeline_ci.basePipelineIndex = -1;

				vk::Result res = Handle(*device).createComputePipelines(pipeline_cache, 1u, &compute_pipeline_ci, allocation_callbacks, &pipeline);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(pipeline, pipeline_ci.name);
#endif
				return make_api_error(pipeline.objectType, res, "Failed to Create");
			}

			FRenderApiErrorInfo VkRenderDevice::CreatePipeline(const FVkRayTracingPipelineCreateInfo& pipeline_ci, vk::Pipeline& pipeline)
			{
				vk::RayTracingShaderGroupCreateInfoKHR shader_group_ci{};
				vk::RayTracingShaderGroupTypeKHR;

				vk::RayTracingPipelineCreateInfoKHR rtx_pipeline_ci{};
				//Handle(*device).createRayTracingPipelinesKHR()
				vk::Result res;// = Handle(*device).createRayTracingPipelinesKHR(pipeline_cache, 1u, &rtx_pipeline_ci, allocation_callbacks, &pipeline);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(pipeline, pipeline_ci.name);
#endif
				return make_api_error(pipeline.objectType, res, "Failed to Create");
			}

			void VkRenderDevice::DestroyPipeline(vk::Pipeline& pipeline)
			{
				Handle(*device).destroyPipeline(pipeline, allocation_callbacks);
			}

			FRenderApiErrorInfo VkRenderDevice::CreateSemaphore(const FVkSemaphoreCreateInfo& semaphore_ci, vk::Semaphore& semaphore)
			{
				vk::Result res = Handle(*device).createSemaphore(&semaphore_ci.ci, allocation_callbacks, &semaphore);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(semaphore, semaphore_ci.name);
#endif
				return make_api_error(semaphore.objectType, res, "Failed to Create");
			}

			FRenderApiErrorInfo VkRenderDevice::CreateFence(const FVkFenceCreateInfo& fence_ci, vk::Fence& fence)
			{
				vk::Result res = Handle(*device).createFence(&fence_ci.ci, allocation_callbacks, &fence);
#ifndef BUILD_WITHOUT_DEBUGGIND
				debug_utils->SetDebugName(fence, fence_ci.name);
#endif
				return make_api_error(fence.objectType, res, "Failed to Create");
			}

			FRenderApiErrorInfo VkRenderDevice::CreateTimestampQuery(u32 count, FVkTimestampQuery& timestamp_query)
			{
				vk::QueryPoolCreateInfo query_pool_ci{};
				query_pool_ci.queryType = vk::QueryType::eTimestamp;
				query_pool_ci.queryCount = count;

				timestamp_query.values.resize(count);

				vk::Result res = Handle(*device).createQueryPool(&query_pool_ci, allocation_callbacks, &timestamp_query.handle);
				Handle(*device).resetQueryPool(timestamp_query.handle, 0u, count);

				return make_api_error(timestamp_query.handle.objectType, res, "Failed to Create");
			}

			void VkRenderDevice::ResetTimestampQuery(FVkTimestampQuery& timestamp_query)
			{
				Handle(*device).resetQueryPool(timestamp_query.handle, 0u, static_cast<u32>(timestamp_query.values.size()));
				timestamp_query.queryIndex = 0u;
			}

			void VkRenderDevice::ResetTimestampQuery(FVkCommandBuffer& command_buffer, FVkTimestampQuery& timestamp_query)
			{
				auto& current_buffer = command_buffer.get();
				current_buffer.resetQueryPool(timestamp_query.handle, 0u, static_cast<u32>(timestamp_query.values.size()));
				timestamp_query.queryIndex = 0u;
			}

			void VkRenderDevice::WriteTimestampQuery(FVkCommandBuffer& command_buffer, vk::PipelineStageFlagBits pipeline_state, FVkTimestampQuery& timestamp_query)
			{
				auto& current_buffer = command_buffer.get();
				current_buffer.writeTimestamp(pipeline_state, timestamp_query.handle, timestamp_query.queryIndex++);
			}

			FRenderApiErrorInfo VkRenderDevice::GetTimestampQueryResults(FVkTimestampQuery& timestamp_query, utl::vector<f32>& results)
			{
				vk::Result res = Handle(*device).getQueryPoolResults(
					timestamp_query.handle,
					0u,
					timestamp_query.queryIndex,
					timestamp_query.values.size() * sizeof(u64),
					timestamp_query.values.data(),
					sizeof(u64),
					vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait);

				auto& physical = instance->GetBestDevice();
				auto& timestamp_frequency = physical.GetProperties().limits.timestampPeriod;

				if (results.size() != timestamp_query.queryIndex / 2ull)
					results.resize(timestamp_query.queryIndex / 2);

				for (u64 idx = 0ull; idx < timestamp_query.queryIndex; idx += 2ull)
				{
					f32 diff = static_cast<f32>(timestamp_query.values[idx + 1] - timestamp_query.values[idx]);
					results[idx / 2ull] = diff * timestamp_frequency / 1000000000.f;
				}

				return make_api_error(timestamp_query.handle.objectType, res, "Failed to get.");
			}

			void VkRenderDevice::DestroyTimestampQuery(FVkTimestampQuery& timestamp_query)
			{
				Handle(*device).destroyQueryPool(timestamp_query.handle);
				timestamp_query.values.clear();
			}

			void VkRenderDevice::GetOccupedDeviceMemory(vk::MemoryHeapFlags memory_flags, u64& total_memory, u64& used_memory)
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

			void VkRenderDevice::SubmitCommandBuffer(const FVkCommandBuffer& command_buffer)
			{
				vk::SubmitInfo submitInfo{};
				submitInfo.commandBufferCount = utl::cast<u32>(command_buffer.handles.size());
				submitInfo.pCommandBuffers = command_buffer.handles.data();

				FVkFenceCreateInfo fence_ci{};
				vk::Fence fence{};
				CreateFence(fence_ci, fence);

				Handle(*device).resetFences(1, &fence);

				auto& queue = device->GetQueueByFlags(GetQueueFlags(command_buffer.mask), 0);
				queue.Submit(submitInfo, fence);
				Handle(*device).waitForFences(1, &fence, VK_TRUE, (std::numeric_limits<u64>::max)());
				Handle(*device).destroyFence(fence, allocation_callbacks);
			}

			vk::Result VkRenderDevice::Present(FVkCommandBuffer& command_buffer)
			{
				vk::Result res = Handle(*device).resetFences(1, &swapchain.inFlightFences[swapchain.currentFrame]);
				check_api_result(make_api_error(vk::ObjectType::eFence, res, "Failed to reset"));

				try
				{
					auto& queue = device->GetQueueByFlags(GetQueueFlags(command_buffer.mask), 0);

					vk::SubmitInfo2 submit_info{};

					vk::SemaphoreSubmitInfo semaphore_si{};
					semaphore_si.semaphore = swapchain.imageAvaliableSemaphores[swapchain.currentFrame];
					semaphore_si.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

					submit_info.pWaitSemaphoreInfos = &semaphore_si;
					submit_info.waitSemaphoreInfoCount = 1;

					vk::SemaphoreSubmitInfo signal_si{};
					signal_si.semaphore = swapchain.renderFinishedSemaphores[swapchain.currentFrame];
					signal_si.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

					submit_info.pSignalSemaphoreInfos = &signal_si;
					submit_info.signalSemaphoreInfoCount = 1;

					auto& current_command_buffer = command_buffer.get();

					vk::CommandBufferSubmitInfo command_buffer_si{};
					command_buffer_si.commandBuffer = current_command_buffer;

					submit_info.pCommandBufferInfos = &command_buffer_si;
					submit_info.commandBufferInfoCount = 1;

					queue.Submit(submit_info, swapchain.inFlightFences[swapchain.currentFrame]);
				}
				catch (vk::SystemError error)
				{
					log_error("failed to submit Draw command buffer!");
				}

				vk::PresentInfoKHR present_info = {};
				present_info.waitSemaphoreCount = 1;
				present_info.pWaitSemaphores = &swapchain.renderFinishedSemaphores[swapchain.currentFrame];

				vk::SwapchainKHR swap_chains[] = { swapchain.handle };
				present_info.swapchainCount = 1;
				present_info.pSwapchains = swap_chains;
				present_info.pImageIndices = &swapchain.currentFrame;

				auto& present_queue = device->GetQueueByPresent(0);
				res = present_queue.Present(present_info);
				check_api_result(make_api_error(vk::ObjectType::eQueue, res, "Failed to present"));

				res = Handle(*device).waitForFences(1, &swapchain.inFlightFences[swapchain.currentFrame], VK_TRUE, (std::numeric_limits<u64>::max)());
				swapchain.currentFrame = (swapchain.currentFrame + 1) % swapchain.framesInFlight;

				return res;
			}

			vk::Result VkRenderDevice::AcquireNextImage(u32& image_index)
			{
				return Handle(*device).acquireNextImageKHR(swapchain.handle, (std::numeric_limits<u64>::max)(), swapchain.imageAvaliableSemaphores[swapchain.currentFrame], nullptr, &image_index);
			}

			void VkRenderDevice::DebugBeginLabel(vk::CommandBuffer& command_buffer, const std::string& name, u32 color)
			{
#ifndef BUILD_WITHOUT_DEBUGGIND
#ifdef ENABLE_TRACY_PROFILING
				TracyVkZone(tracy_context.at(swapchain.currentFrame), command_buffer, name.c_str());
#endif
				command_buffer.beginDebugUtilsLabelEXT(make_debug_label(name.c_str(), color));
#endif
			}

			void VkRenderDevice::DebugInsertLabel(vk::CommandBuffer& command_buffer, const std::string& name, u32 color)
			{
#ifndef BUILD_WITHOUT_DEBUGGIND
				command_buffer.insertDebugUtilsLabelEXT(make_debug_label(name.c_str(), color));
#endif
			}

			void VkRenderDevice::EndLabel(vk::CommandBuffer& command_buffer)
			{
#ifndef BUILD_WITHOUT_DEBUGGIND
				command_buffer.endDebugUtilsLabelEXT();
#endif
			}

			void VkRenderDevice::DebugBeginLabel(vk::Queue& queue, const std::string& name, u32 color)
			{
#ifndef BUILD_WITHOUT_DEBUGGIND
				queue.beginDebugUtilsLabelEXT(make_debug_label(name.c_str(), color));
#endif
			}

			void VkRenderDevice::DebugInsertLabel(vk::Queue& queue, const std::string& name, u32 color)
			{
#ifndef BUILD_WITHOUT_DEBUGGIND
				queue.insertDebugUtilsLabelEXT(make_debug_label(name.c_str(), color));
#endif
			}

			void VkRenderDevice::EndLabel(vk::Queue& queue)
			{
#ifndef BUILD_WITHOUT_DEBUGGIND
				queue.endDebugUtilsLabelEXT();
#endif
			}

			void VkRenderDevice::InitProfiler(const FVkCommandPool& command_pool)
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

			vk::Format VkRenderDevice::FindSupportedFormat(const utl::vector<vk::Format>& candidates, bool depth_only)
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

			const vk::DynamicLoader& VkRenderDevice::GetDynamicLoader() const
			{
				return dl;
			}

			const vk::AllocationCallbacks* VkRenderDevice::GetAllocationCallbacks() const
			{
				return allocation_callbacks;
			}

			const vk::SurfaceKHR& VkRenderDevice::GetSurface() const
			{
				return surface;
			}

			const vk::SampleCountFlagBits VkRenderDevice::GetMultisamplingCount() const
			{
				return required_sample_count;
			}

			Instance& VkRenderDevice::GetInstance() const
			{
				return *instance;
			}

			PhysicalDevice& VkRenderDevice::GetPhysicalDevice() const
			{
				return instance->GetBestDevice();
			}

			Device& VkRenderDevice::GetLogicalDevice() const
			{
				return *device;
			}

			DebugUtils& VkRenderDevice::GetDebugUtils() const
			{
				return *debug_utils;
			}

			Allocator& VkRenderDevice::GetAllocator() const
			{
				return *allocator;
			}

			vma::Allocator& VkRenderDevice::GetMemoryAllocator()
			{
				return Handle(*allocator);
			}

			const vk::PipelineCache& VkRenderDevice::GetPipelineCache() const
			{
				return pipeline_cache;
			}

			utl::vector<char> VkRenderDevice::GetPipelineCacheData() const
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

			const std::vector<vk::SurfaceFormatKHR>& VkRenderDevice::GetSurfacePriorityList() const
			{
				return surface_priority_list;
			}

			const FSwapchain& VkRenderDevice::GetSwapchain() const
			{
				return swapchain;
			}

			void VkRenderDevice::RegisterAllocationCallbacks()
			{
				allocation_callbacks = new vk::AllocationCallbacks();
				memset(allocation_callbacks, 0, sizeof(vk::AllocationCallbacks));

				allocation_callbacks->pfnAllocation = (PFN_vkAllocationFunction)(&vk_malloc);
				allocation_callbacks->pfnReallocation = (PFN_vkReallocationFunction)(&vk_realloc);
				allocation_callbacks->pfnFree = (PFN_vkFreeFunction)(&vk_free);
				allocation_callbacks->pfnInternalAllocation = (PFN_vkInternalAllocationNotification)(&vk_malloc_notification);
				allocation_callbacks->pfnInternalFree = (PFN_vkInternalFreeNotification)(&vk_free_notification);
			}

			void VkRenderDevice::EnableDeviceFeatures(PhysicalDevice& device)
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

			FRenderApiErrorInfo VkRenderDevice::CreatePipelineCache(const FRenderContextCreateInfo& context_ci)
			{
				vk::PipelineCacheCreateInfo pipeline_cache_ci{};
				pipeline_cache_ci.initialDataSize = utl::cast<u32>(context_ci.pipelineCache.size());
				pipeline_cache_ci.pInitialData = context_ci.pipelineCache.data();
				return make_api_error(vk::ObjectType::ePipelineCache, device->CreateObject(pipeline_cache_ci, pipeline_cache), "Failed to Create");
			}

			FRenderApiErrorInfo VkRenderDevice::CreateSwapchain(u32 surface_width, u32 surface_height)
			{
				auto& physical_device = instance->GetBestDevice();

				vk::SwapchainKHR old_swapchain{ swapchain.handle };
				swapchain.handle = nullptr;

				auto surface_caps = Handle(physical_device).getSurfaceCapabilitiesKHR(surface);
				auto surface_formats = Handle(physical_device).getSurfaceFormatsKHR(surface);
				auto present_modes = Handle(physical_device).getSurfacePresentModesKHR(surface);

				auto surface_format = choice_surface_format(surface_formats);
				auto present_mode = choice_surface_present_mode(present_modes);
				swapchain.extent = choice_swapchain_extent(surface_caps, surface_width, surface_height);
				swapchain.format = surface_format.format;

				u32 image_count{ swapchain.framesInFlight };
				if (surface_caps.maxImageCount > 0 && image_count > surface_caps.maxImageCount)
					image_count = surface_caps.maxImageCount;
				else if (image_count < surface_caps.minImageCount)
					image_count = surface_caps.minImageCount;
				swapchain.framesInFlight = image_count;

				vk::SwapchainCreateInfoKHR swapchain_ci{};
				swapchain_ci.surface = surface;
				swapchain_ci.minImageCount = image_count;
				swapchain_ci.imageFormat = surface_format.format;
				swapchain_ci.imageColorSpace = surface_format.colorSpace;
				swapchain_ci.imageExtent = swapchain.extent;
				swapchain_ci.imageArrayLayers = 1u;
				swapchain_ci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

				if (surface_caps.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc)
					swapchain_ci.imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
				if (surface_caps.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst)
					swapchain_ci.imageUsage |= vk::ImageUsageFlagBits::eTransferDst;

				// TODO: Complete this one
				//auto& graphics_family = device->GetQueueByFlags(vk::QueueFlagBits::eGraphics, 0);
				//auto& present_family = device->GetQueueByPresent(0);
				//
				//if (graphics_family.GetFamilyIndex() != present_family.GetFamilyIndex())
				//{
				//	swapchain_ci.imageSharingMode = vk::SharingMode::eConcurrent;
				//	swapchain_ci.queueFamilyIndexCount = utl::cast<u32>(device->GetQueueCount());
				//	swapchain_ci.pQueueFamilyIndices = physical_device.queueFamilies.uniqueFamilies.data();
				//}
				//else
				swapchain_ci.imageSharingMode = vk::SharingMode::eExclusive;

				swapchain_ci.preTransform = surface_caps.currentTransform;
				swapchain_ci.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
				swapchain_ci.presentMode = present_mode;
				swapchain_ci.clipped = VK_TRUE;
				swapchain_ci.oldSwapchain = old_swapchain;

				vk::Result res = device->CreateObject(swapchain_ci, swapchain.handle);
				check_api_result(make_api_error(vk::ObjectType::eSwapchainKHR, res, "Failed to Create"));

				swapchain.images.resize(image_count);
				res = Handle(*device).getSwapchainImagesKHR(swapchain.handle, &image_count, swapchain.images.data());
				check_api_result(make_api_error(vk::ObjectType::eSwapchainKHR, res, "Failed to Create images"));

#ifndef BUILD_WITHOUT_DEBUGGIND
				for (u64 idx = 0ull; idx < swapchain.images.size(); ++idx)
				{
					const auto& image = swapchain.images[idx];
					auto name = std::format("swapchain_image_{}", idx);
					debug_utils->SetDebugName(image, name);
				}
#endif

				if (old_swapchain)
					Handle(*device).destroySwapchainKHR(old_swapchain, allocation_callbacks);

				// Create swapchain image views
				{
					swapchain.imageViews.resize(image_count);

					FVkImageViewCreateInfo image_view_ci{};
					image_view_ci.ci.viewType = vk::ImageViewType::e2D;
					image_view_ci.ci.format = swapchain.format;
					image_view_ci.ci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
					image_view_ci.ci.subresourceRange.baseMipLevel = 0;
					image_view_ci.ci.subresourceRange.levelCount = 1;
					image_view_ci.ci.subresourceRange.baseArrayLayer = 0;
					image_view_ci.ci.subresourceRange.layerCount = 1;
					image_view_ci.ci.components.r = vk::ComponentSwizzle::eR;
					image_view_ci.ci.components.g = vk::ComponentSwizzle::eG;
					image_view_ci.ci.components.b = vk::ComponentSwizzle::eB;
					image_view_ci.ci.components.a = vk::ComponentSwizzle::eA;

					for (u32 idx = 0u; idx < image_count; ++idx)
					{
#ifndef BUILD_WITHOUT_DEBUGGIND
						image_view_ci.name = std::format("frame #{}", idx);
#endif
						image_view_ci.ci.image = swapchain.images[idx];
						check_api_result(CreateImageView(image_view_ci, swapchain.imageViews[idx]));
					}
				}

				// Create swapchain sync objects
				{
					swapchain.imageAvaliableSemaphores.resize(image_count);
					swapchain.renderFinishedSemaphores.resize(image_count);
					swapchain.inFlightFences.resize(image_count);

					FVkSemaphoreCreateInfo ia_semaphore_ci{};
					FVkSemaphoreCreateInfo rf_semaphore_ci{};
					FVkFenceCreateInfo fence_ci{};
					fence_ci.ci.flags = vk::FenceCreateFlagBits::eSignaled;
					for (u64 idx = 0ull; idx < image_count; ++idx)
					{
#ifndef BUILD_WITHOUT_DEBUGGIND
						ia_semaphore_ci.name = std::format("image avaliable #{}", idx);
						rf_semaphore_ci.name = std::format("render finished #{}", idx);
						fence_ci.name = std::format("frame #{}", idx);
#endif
						check_api_result(CreateSemaphore(ia_semaphore_ci, swapchain.imageAvaliableSemaphores[idx]));
						check_api_result(CreateSemaphore(rf_semaphore_ci, swapchain.renderFinishedSemaphores[idx]));
						check_api_result(CreateFence(fence_ci, swapchain.inFlightFences[idx]));
					}
				}

				return make_api_error(vk::ObjectType::eSwapchainKHR, vk::Result::eSuccess);
			}

			void VkRenderDevice::CleanupSwapchain()
			{
				for (u64 idx = 0ull; idx < swapchain.framesInFlight; ++idx)
				{
					Handle(*device).destroyFence(swapchain.inFlightFences[idx], allocation_callbacks);
					Handle(*device).destroyImageView(swapchain.imageViews[idx], allocation_callbacks);
					Handle(*device).destroySemaphore(swapchain.imageAvaliableSemaphores[idx], allocation_callbacks);
					Handle(*device).destroySemaphore(swapchain.renderFinishedSemaphores[idx], allocation_callbacks);
				}
			}
		}
	}
}