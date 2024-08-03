#pragma once

#include <engine/interface/render/RenderInterface.h>
#include "VkResourceStructures.h"

#include <format>

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

namespace tracy
{
	class VkCtx;
}

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			// =================================================================================
			// =================================CVkRenderDevice================================
			// =================================================================================
			class CVkRenderDevice
			{
			public:
				friend class CVkRenderContext;

				~CVkRenderDevice();

				void Create(const FRenderContextCreateInfo& context_ci);

				// VkCommandPool
				FRenderApiErrorInfo CreateCommandPool(const FVkCommandPoolCreateInfo& command_pool_ci, FVkCommandPool& command_pool);
				void DestroyCommandPool(FVkCommandPool& command_pool);

				// VkCommandBuffer
				FRenderApiErrorInfo CreateCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, const FVkCommandPool& command_pool, FVkCommandBuffer& command_buffer);
				FVkCommandBuffer& GetCommandBuffer(const FVkGetCommandBufferInfo& get_info);
				void DestroyCommandBuffer(const FVkCommandPool& command_pool, FVkCommandBuffer& command_buffer);
				void BeginCommandBuffer(FVkCommandBuffer& command_buffer, vk::CommandBufferUsageFlags usage, u32 image_index);
				void EndCommandBuffer(FVkCommandBuffer& command_buffer);

				// VkImage
				FRenderApiErrorInfo CreateImage(const FVkImageCreateInfo& image_ci, FVkImage& image);
				void DestroyImage(FVkImage& image);

				// VkImageView
				FRenderApiErrorInfo CreateImageView(const FVkImageViewCreateInfo& image_view_ci, vk::ImageView& image_view);
				void DestroyImageView(vk::ImageView& image_view);

				FRenderApiErrorInfo CreateSampler(const FVkSamplerCreateInfo& sampler_ci, vk::Sampler& sampler);
				void DestroySampler(vk::Sampler& sampler);

				// VkBuffer
				FRenderApiErrorInfo CreateBuffer(const FVkBufferCreateInfo& buffer_ci, FVkBuffer& buffer);
				void DestroyBuffer(FVkBuffer& buffer);
				FRenderApiErrorInfo MapBufferMemory(FVkBuffer& buffer, void** mappedMemory);
				void UnmapBufferMemory(FVkBuffer& buffer);
				void FlushBufferMemory(const FVkBuffer& buffer, vk::DeviceSize size, vk::DeviceSize offset);

				// VkDescriptorSetLayout
				FRenderApiErrorInfo CreateDescriptorSetLayout(const FVkDescriptorSetLayoutCreateInfo& layout_ci, vk::DescriptorSetLayout& descriptor_set_layout);
				void DestroyDescriptorSetLayout(vk::DescriptorSetLayout& descriptor_set_layout);

				FRenderApiErrorInfo CreateDescriptorPool(const FVkDescriptorPoolCreateInfo& pool_ci, FVkDescriptorPool& descriptor_pool);
				FRenderApiErrorInfo GrowDescriptorPool(const FVkDescriptorPoolCreateInfo& pool_ci, FVkDescriptorPool& descriptor_pool);
				void ResetDescriptorPool(FVkDescriptorPool& descriptor_pool);
				void DestroyDescriptorPool(FVkDescriptorPool& descriptor_pool);

				FRenderApiErrorInfo CreateDescriptorSet(const FVkDescriptorSetCreateInfo& set_ci, FVkDescriptorSet& descriptor_set);
				void DestroyDescriptorSet(const FVkDescriptorPool& descriptor_pool, FVkDescriptorSet& descriptor_set);

				// VkShaderModule
				FRenderApiErrorInfo CreateShaderModule(const FVkShaderModuleCreateInfo& shader_module_ci, vk::ShaderModule& shader_module);
				void DestroyShaderModule(vk::ShaderModule& shader_module);

				// VkPipelineLayout
				FRenderApiErrorInfo CreatePipelineLayout(const FVkPipelineLayoutCreateInfo& pipeline_layout_ci, vk::PipelineLayout& pipeline_layout);
				void DestroyPipelineLayout(vk::PipelineLayout& pipeline_layout);

				// VkPipeline
				FRenderApiErrorInfo CreatePipeline(const FVkGraphicsPipelineCreateInfo& pipeline_ci, vk::Pipeline& pipeline);
				FRenderApiErrorInfo CreatePipeline(const FVkComputePipelineCreateInfo& pipeline_ci, vk::Pipeline& pipeline);
				FRenderApiErrorInfo CreatePipeline(const FVkRayTracingPipelineCreateInfo& pipeline_ci, vk::Pipeline& pipeline);
				void DestroyPipeline(vk::Pipeline& shader_object);

				FRenderApiErrorInfo CreateSemaphore(const FVkSemaphoreCreateInfo& semaphore_ci, vk::Semaphore& semaphore);
				FRenderApiErrorInfo CreateFence(const FVkFenceCreateInfo& fence_ci, vk::Fence& fence);

				FRenderApiErrorInfo CreateTimestampQuery(u32 count, FVkTimestampQuery& timestamp_query);
				void ResetTimestampQuery(FVkTimestampQuery& timestamp_query);
				void ResetTimestampQuery(FVkCommandBuffer& command_buffer, FVkTimestampQuery& timestamp_query);
				void WriteTimestampQuery(FVkCommandBuffer& command_buffer, vk::PipelineStageFlagBits pipeline_state, FVkTimestampQuery& timestamp_query);
				FRenderApiErrorInfo GetTimestampQueryResults(FVkTimestampQuery& timestamp_query, utl::vector<f32>& results);
				void DestroyTimestampQuery(FVkTimestampQuery& timestamp_query);

				void GetOccupedDeviceMemory(vk::MemoryHeapFlags memory_flags, u64& total_memory, u64& used_memory);

				void SubmitCommandBuffer(const FVkCommandBuffer& command_buffer);
				vk::Result Present(FVkCommandBuffer& command_buffer);
				vk::Result AcquireNextImage(u32& image_index);

				// Debug markers
				void DebugBeginLabel(vk::CommandBuffer& command_buffer, const std::string& name, u32 color);
				void DebugInsertLabel(vk::CommandBuffer& command_buffer, const std::string& name, u32 color);
				void EndLabel(vk::CommandBuffer& command_buffer);

				void DebugBeginLabel(vk::Queue& queue, const std::string& name, u32 color);
				void DebugInsertLabel(vk::Queue& queue, const std::string& name, u32 color);
				void EndLabel(vk::Queue& queue);

				template<class _Ty>
				void DebugSetObjectName(const _Ty& object, const std::string& name)
				{
#ifndef BUILD_WITHOUT_DEBUGGIND
					if (!name.empty())
					{
						vk::DebugUtilsObjectNameInfoEXT object_name_info{};
						object_name_info.objectType = object.objectType;
						object_name_info.objectHandle = reinterpret_cast<u64>(static_cast<_Ty::CType>(object));

						auto formatted_debug_object_name = std::format("{}:{} | {:x}", vk::to_string(object.objectType), name, object_name_info.objectHandle);
						object_name_info.pObjectName = formatted_debug_object_name.c_str();

						m_logicalDevice.handle.setDebugUtilsObjectNameEXT(object_name_info);
					}
#endif
				}

				template<class _Ty>
				void DebugSetObjectTag(const _Ty& object, u64 name, const void* data, u64 size)
				{
#ifndef BUILD_WITHOUT_DEBUGGIND
					vk::DebugUtilsObjectTagInfoEXT object_tag_info{};
					object_tag_info.objectType = object.objectType;
					object_tag_info.objectHandle = reinterpret_cast<u64>(static_cast<_Ty::CType>(object));
					object_tag_info.tagName = name;
					object_tag_info.pTag = data;
					object_tag_info.tagSize = size;

					m_logicalDevice.handle.setDebugUtilsObjectTagEXT(object_tag_info);
#endif
				}

				void InitProfiler(const FVkCommandPool& command_pool);

				// Getters
				vk::Format FindSupportedFormat(const utl::vector<vk::Format>& candidates, bool depth_only);

				const vk::DynamicLoader& GetDynamicLoader() const;
				const vk::AllocationCallbacks* GetAllocationCallbacks() const;

				const vk::Instance& GetInstance() const;
				const vk::DebugUtilsMessengerEXT& GetDebugMessanger() const;
				const vk::DebugReportCallbackEXT& GetDebugReport() const;
				const vk::SurfaceKHR& GetSurface() const;
				
				FVkPhysicalDevice& GetPhysicalDevice();

				const vk::SampleCountFlagBits GetMultisamplingCount() const;
				const FVkLogicalDevice& GetLogicalDevice() const;

				vk::Queue& GetQueue(EQueueTypeFlagBits flag);

				vma::Allocator& GetMemoryAllocator();

				const vk::PipelineCache& GetPipelineCache() const;
				utl::vector<char> GetPipelineCacheData() const;

				const FSwapchain& GetSwapchain() const;
			protected:
				void RegisterAllocationCallbacks();
				FRenderApiErrorInfo CreateInstance(const FRenderContextCreateInfo& context_ci);
				void CreateDebugMessanger(const FRenderContextCreateInfo& context_ci);
				FRenderApiErrorInfo CreateSurface(const FRenderContextCreateInfo& context_ci);
				void SelectQueueFamilies(FVkPhysicalDevice& device_info);
				FRenderApiErrorInfo SelectPhysicalDevice(const FRenderContextCreateInfo& context_ci);
				void PrepareRequestedLogicalDeviceFeatures();
				FRenderApiErrorInfo CreateLogicalDevice(const FRenderContextCreateInfo& context_ci);
				void CreateMemoryAllocator(const FRenderContextCreateInfo& context_ci);
				FRenderApiErrorInfo CreatePipelineCache(const FRenderContextCreateInfo& context_ci);
				FRenderApiErrorInfo CreateSwapchain(u32 surface_width, u32 surface_height);

				void CleanupSwapchain();

				template<class _Ty>
				_Ty& RequestExtensionFeatures()
				{
					auto structure_type = _Ty::structureType;

					auto& physical_device = GetPhysicalDevice();
					auto& extension_features = m_logicalDevice.extensionFeatures;

					auto found_feature = extension_features.find(structure_type);
					if (found_feature != extension_features.end())
						return *utl::cast<_Ty*>(found_feature->second.get());

					vk::PhysicalDeviceFeatures2KHR physical_device_features2{};
					_Ty extension{};
					physical_device_features2.pNext = &extension;

					physical_device.handle.getFeatures2KHR(&physical_device_features2);

					extension_features.insert({ structure_type, std::make_shared<_Ty>(extension) });

					auto* extension_ptr = utl::cast<_Ty*>(extension_features[structure_type].get());

					if (m_logicalDevice.lastExtensionFeature)
						extension_ptr->pNext = m_logicalDevice.lastExtensionFeature;
					m_logicalDevice.lastExtensionFeature = extension_ptr;

					return *extension_ptr;
				}
			private:
				vk::DynamicLoader m_dynamicLoader;
				vk::AllocationCallbacks* m_allocationCallbacks{ nullptr };

				vk::Instance m_vkInstance{ nullptr };
				vk::DebugUtilsMessengerEXT m_debugMessenger{ nullptr };
				vk::DebugReportCallbackEXT m_debugReport{ nullptr };
				vk::SurfaceKHR m_vkSurface{ nullptr };
				utl::vector<std::string> m_avaliableLayers{};

				i32 m_selectedPhysicalDevice{ -1 };
				utl::vector<FVkPhysicalDevice> m_physicalDevices{};

				vk::SampleCountFlagBits m_requiredSampleCount{ vk::SampleCountFlagBits::e1 };
				FVkLogicalDevice m_logicalDevice{};

				vma::Allocator m_memoryAllocator{ nullptr };

				vk::PipelineCache m_pipelineCache{ nullptr };
				FSwapchain m_swapchain{};

#ifdef ENABLE_TRACY_PROFILING
				utl::vector<tracy::VkCtx*> m_tracyContext;
				tracy::VkCtx* m_presentTracyContext;
#endif
			};
		}
	}
}