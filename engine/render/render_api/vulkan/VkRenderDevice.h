#pragma once

#include <engine/interface/render/RenderInterface.h>
#include <engine/render/render_api/vulkan/VkResourceStructures.h>

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
			class Allocator;
			class Allocation;

			/**
			 * @brief A class that represents all device functions
			 *
			 * This class is responsible for creating all vulkan objects associated with vk::Device. 
			 */
			class RenderDevice
			{
			public:
				friend class VkRenderBackend;

				~RenderDevice();

				/**
				 * @brief Add a sample-specific device extension
				 * @param extension The extension name
				 * @param optional (Optional) Whether the extension is optional
				 */
				void AddDeviceExtension(const char* extension);

				void Create(const FRenderContextCreateInfo& context_ci);

				/**
				 * @brief Creates buffer handle and allocates memory for it
				 * @param allocation Allocation handle that contains vma allocation
				 * @param create_info Vulkan buffer create info
				 * @param debug_name Debug name for handle
				 * @return Vulkan buffer handle
				 */
				vk::Buffer CreateBuffer(Allocation& allocation, vk::BufferCreateInfo const& create_info, const std::string& debug_name = "") const;

				/**
				 * @brief Returns virtual buffer address of buffer on gpu side
				 * @param handle Vulkan buffer handle
				 * @return Virtual address to buffer memory
				 */
				uint64_t GetBufferDeviceAddress(const vk::Buffer& handle) const;

				/**
				 * @brief Destroys buffer handle and deallocates buffer memory
				 * @param allocation Allocation handle that contains vma allocation
				 * @param handle Vulkan buffer handle
				 */
				void Destroy(Allocation& allocation, vk::Buffer& handle) const;

				/**
				 * @brief Creates image handle and allocates memory for it
				 * @param allocation Allocation handle that contains vma allocation
				 * @param create_info Vulkan image create info
				 * @param debug_name Debug name for handle
				 * @return Vulkan image handle
				 */
				vk::Image CreateImage2(Allocation& allocation, vk::ImageCreateInfo const& create_info, const std::string& debug_name = "") const;

				/**
				 * @brief Destroys image handle and deallocates it's memory
				 * @param allocation Allocation handle that contains vma allocation
				 * @param handle Vulkan image handle
				 */
				void Destroy(Allocation& allocation, vk::Image& handle) const;

				/**
				 * @brief Creates image view handle
				 * @param create_info Vulkan image view create info
				 * @param debug_name Debug name for handle
				 * @return Vulkan image view handle
				 */
				vk::ImageView CreateImageView(const vk::ImageViewCreateInfo& create_info, const std::string& debug_name = "") const;

				/**
				 * @brief Destroys image view handle
				 * @param handle Vulkan image view handle
				 */
				void Destroy(vk::ImageView& image_view) const;

				/**
				 * @brief Creates swapchainKHR handle
				 * @param create_info Vulkan swapchainKHR create info
				 * @param debug_name Debug name for handle
				 * @return Vulkan swapchainKHR handle
				 */
				vk::SwapchainKHR CreateSwapchain2(const vk::SwapchainCreateInfoKHR& create_info, const std::string& debug_name = "") const;

				/**
				 * @brief Requests vulkan images from swapchain handle
				 * @param handle Vulkan swapchainKHR handle
				 * @param images Mutable vector of vk image handles
				 * @param image_count Mutable number of swapchain image count
				 * @param debug_name Per-image debug name pattern
				 */
				void GetSwapchainImages(const vk::SwapchainKHR& handle, std::vector<vk::Image>& images, uint32_t& image_count, const std::string& debug_name = "");

				/**
				 * @brief Destroys swapchainKHR handle
				 * @param handle Vulkan swapchainKHR handle
				 */
				void Destroy(vk::SwapchainKHR& handle) const;

				/**
				 * @brief Creates pipeline cache handle
				 * @param create_info Vulkan pipeline cache create info
				 * @param debug_name Debug name for handle
				 * @return Vulkan pipeline cache handle
				 */
				vk::PipelineCache CreatePipelineCache2(const vk::PipelineCacheCreateInfo& create_info, const std::string& debug_name = "") const;

				/**
				 * @brief Destroys pipeline cache handle
				 * @param handle Vulkan pipeline cache handle
				 */
				void Destroy(vk::PipelineCache& handle) const;

				/**
				 * @brief Creates sampler handle
				 * @param create_info Vulkan sampler create info
				 * @param debug_name Debug name for handle
				 * @return Vulkan sampler handle
				 */
				vk::Sampler CreateSampler2(const vk::SamplerCreateInfo& create_info, const std::string& debug_name = "") const;

				/**
				 * @brief Destroys sampler handle
				 * @param handle Vulkan sampler handle
				 */
				void Destroy(vk::Sampler& handle) const;

				/**
				 * @brief Creates semaphore handle
				 * @param create_info Vulkan semaphore create info
				 * @param debug_name Debug name for handle
				 * @return Vulkan semaphore handle
				 */
				vk::Semaphore CreateSemaphore2(const vk::SemaphoreCreateInfo& create_info, const std::string& debug_name = "") const;

				/**
				 * @brief Destroys semaphore handle
				 * @param handle Vulkan semaphore handle
				 */
				void Destroy(vk::Semaphore& handle) const;

				/**
				 * @brief Creates fence handle
				 * @param create_info Vulkan fence create info
				 * @param debug_name Debug name for handle
				 * @return Vulkan fence handle
				 */
				vk::Fence CreateFence2(const vk::FenceCreateInfo& create_info, const std::string& debug_name = "") const;

				/**
				 * @brief Destroys fence handle
				 * @param handle Vulkan fence handle
				 */
				void Destroy(vk::Fence& handle) const;

				// VkCommandPool
				FRenderApiErrorInfo CreateCommandPool(const FVkCommandPoolCreateInfo& command_pool_ci, FVkCommandPool& command_pool);
				void DestroyCommandPool(FVkCommandPool& command_pool);

				// VkCommandBuffer
				FRenderApiErrorInfo CreateCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, const FVkCommandPool& command_pool, FVkCommandBuffer& command_buffer);
				FVkCommandBuffer& GetCommandBuffer(const FVkGetCommandBufferInfo& get_info);
				void DestroyCommandBuffer(const FVkCommandPool& command_pool, FVkCommandBuffer& command_buffer);
				void BeginCommandBuffer(FVkCommandBuffer& command_buffer, vk::CommandBufferUsageFlags usage, u32 image_index);
				void EndCommandBuffer(FVkCommandBuffer& command_buffer);

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
				Allocator& GetAllocator() const;

				vma::Allocator& GetMemoryAllocator();

				const vk::PipelineCache& GetPipelineCache() const;
				utl::vector<char> GetPipelineCacheData() const;

				const std::vector<vk::SurfaceFormatKHR>& GetSurfacePriorityList() const;

				const FSwapchain& GetSwapchain() const;
			protected:
				void RegisterAllocationCallbacks();
				void EnableDeviceFeatures(PhysicalDevice& device);
				FRenderApiErrorInfo CreatePipelineCache(const FRenderContextCreateInfo& context_ci);
				FRenderApiErrorInfo CreateSwapchain(u32 surface_width, u32 surface_height);

				void CleanupSwapchain();
			private:
				vk::DynamicLoader dl;
				vk::AllocationCallbacks* allocation_callbacks{ nullptr };

				/** @brief Set of device extensions to be enabled for this example and whether they are optional (must be set in the derived constructor) */
				std::vector<const char*> device_extensions;

				std::unique_ptr<Instance> instance;

				vk::SurfaceKHR surface{ nullptr };

				std::unique_ptr<DebugUtils> debug_utils;

				std::unique_ptr<Device> device;

				std::unique_ptr<Allocator> allocator;

				vk::SampleCountFlagBits required_sample_count{ vk::SampleCountFlagBits::e1 };

				vk::PipelineCache pipeline_cache{ nullptr };
				FSwapchain swapchain{};

				/**
				 * @brief A list of surface formats in order of priority (vector[0] has high priority, vector[size-1] has low priority)
				 */
				std::vector<vk::SurfaceFormatKHR> surface_priority_list = {
					{vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
					{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear} };

#ifdef ENABLE_TRACY_PROFILING
				utl::vector<tracy::VkCtx*> tracy_context;
				tracy::VkCtx* present_tracy_context;
#endif
			};
		}
	}
}