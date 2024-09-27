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
			class Instance;
			class PhysicalDevice;
			class DebugUtils;
			class Device;

			// =================================================================================
			// =================================CVkRenderDevice================================
			// =================================================================================
			class CVkRenderDevice
			{
			public:
				friend class VkRenderBackend;

				~CVkRenderDevice();

				/**
				 * @brief Add a sample-specific device extension
				 * @param extension The extension name
				 * @param optional (Optional) Whether the extension is optional
				 */
				void AddDeviceExtension(const char* extension);

				/**
				 * @brief Add a sample-specific instance extension
				 * @param extension The extension name
				 * @param optional (Optional) Whether the extension is optional
				 */
				void AddInstanceExtension(const char* extension);

				/**
				 * @brief Add a sample-specific layer setting
				 * @param layerSetting The layer setting
				 */
				void AddLayerSetting(vk::LayerSettingEXT const& layerSetting);

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

				void InitProfiler(const FVkCommandPool& command_pool);

				// Getters
				vk::Format FindSupportedFormat(const utl::vector<vk::Format>& candidates, bool depth_only);

				const vk::DynamicLoader& GetDynamicLoader() const;
				const vk::AllocationCallbacks* GetAllocationCallbacks() const;

				const vk::SurfaceKHR& GetSurface() const;

				const vk::SampleCountFlagBits GetMultisamplingCount() const;
				Instance& GetInstance() const;
				PhysicalDevice& GetPhysicalDevice() const;
				Device& GetLogicalDevice() const;
				DebugUtils& GetDebugUtils() const;

				vma::Allocator& GetMemoryAllocator();

				const vk::PipelineCache& GetPipelineCache() const;
				utl::vector<char> GetPipelineCacheData() const;

				const FSwapchain& GetSwapchain() const;
			protected:
				void RegisterAllocationCallbacks();
				void EnableDeviceFeatures(PhysicalDevice& device);
				void CreateMemoryAllocator(const FRenderContextCreateInfo& context_ci);
				FRenderApiErrorInfo CreatePipelineCache(const FRenderContextCreateInfo& context_ci);
				FRenderApiErrorInfo CreateSwapchain(u32 surface_width, u32 surface_height);

				void CleanupSwapchain();
			private:
				vk::DynamicLoader m_dynamicLoader;
				vk::AllocationCallbacks* m_allocationCallbacks{ nullptr };

				/** @brief Set of device extensions to be enabled for this example and whether they are optional (must be set in the derived constructor) */
				std::vector<const char*> device_extensions;

				/** @brief Set of instance extensions to be enabled for this example and whether they are optional (must be set in the derived constructor) */
				std::vector<const char*> instance_extensions;

				/** @brief Validation layers */
				std::vector<const char*> validation_layers;

				/** @brief Vector of layer settings to be enabled for this example (must be set in the derived constructor) */
				std::vector<vk::LayerSettingEXT> layer_settings;

				std::unique_ptr<Instance> instance;

				vk::SurfaceKHR surface{ nullptr };

				std::unique_ptr<DebugUtils> debug_utils;

				std::unique_ptr<Device> device;

				vk::SampleCountFlagBits m_requiredSampleCount{ vk::SampleCountFlagBits::e1 };

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