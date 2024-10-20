#pragma once

#include <engine/interface/render/RenderInterface.h>
#include <ResourceStructures.h>

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
				friend class RenderBackend;

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
				vk::Image CreateImage(Allocation& allocation, vk::ImageCreateInfo const& create_info, const std::string& debug_name = "") const;

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
				vk::SwapchainKHR CreateSwapchain(const vk::SwapchainCreateInfoKHR& create_info, const std::string& debug_name = "") const;

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
				vk::PipelineCache CreatePipelineCache(const vk::PipelineCacheCreateInfo& create_info, const std::string& debug_name = "") const;

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
				vk::Sampler CreateSampler(const vk::SamplerCreateInfo& create_info, const std::string& debug_name = "") const;

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
				vk::Semaphore CreateSemaphore(const vk::SemaphoreCreateInfo& create_info, const std::string& debug_name = "") const;

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
				vk::Fence CreateFence(const vk::FenceCreateInfo& create_info, const std::string& debug_name = "") const;

				vk::Result WaitForFences(vk::Fence* handles, uint32_t fence_count, vk::Bool32 wait_for_all = VK_TRUE, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;
				vk::Result WaitForFences(std::span<vk::Fence> handles, vk::Bool32 wait_for_all = VK_TRUE, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;
				vk::Result ResetFences(vk::Fence* handles, uint32_t fence_count) const;
				vk::Result ResetFences(std::span<vk::Fence> handles) const;

				/**
				 * @brief Destroys fence handle
				 * @param handle Vulkan fence handle
				 */
				void Destroy(vk::Fence& handle) const;

				vk::Pipeline CreateGraphicsPipeline(const vk::GraphicsPipelineCreateInfo& create_info, const std::string& debug_name = "") const;
				vk::Pipeline CreateComputePipeline(const vk::ComputePipelineCreateInfo& create_info, const std::string& debug_name = "") const;
				void Destroy(vk::Pipeline& handle) const;

				vk::CommandPool CreateCommandPool(const vk::CommandPoolCreateInfo& create_info, const std::string& debug_name = "") const;
				void ResetCommandPool(vk::CommandPool& handle) const;
				void AllocateCommandBuffers(const vk::CommandBufferAllocateInfo& allocate_info, vk::CommandBuffer* command_buffers) const;
				void FreeCommandBuffers(vk::CommandPool& command_pool, vk::CommandBuffer* command_buffers, uint32_t command_buffer_count);
				void FreeCommandBuffers(vk::CommandPool& command_pool, std::span<vk::CommandBuffer> command_buffers);
				void Destroy(vk::CommandPool& handle) const;

				vk::DescriptorSetLayout CreateDescriptorSetLayout(const vk::DescriptorSetLayoutCreateInfo& create_info, const std::string& debug_name = "") const;
				void Destroy(vk::DescriptorSetLayout& handle) const;

				vk::DescriptorPool CreateDescriptorPool(const vk::DescriptorPoolCreateInfo& create_info, const std::string& debug_name = "") const;
				void ResetDescriptorPool(vk::DescriptorPool& handle, vk::DescriptorPoolResetFlags flags = {}) const;
				void Destroy(vk::DescriptorPool& handle) const;

				void AllocateDescriptorSet(const vk::DescriptorSetAllocateInfo& allocate_info, vk::DescriptorSet* descriptor_sets, const std::string& debug_name = "") const;
				void FreeDescriptorSets(vk::DescriptorPool& pool, vk::DescriptorSet* descriptor_sets, uint32_t descriptor_set_count) const;
				void FreeDescriptorSets(vk::DescriptorPool& pool, std::span<vk::DescriptorSet> descriptor_sets) const;

				vk::ShaderModule CreateShaderModule(const vk::ShaderModuleCreateInfo& create_info, const std::string& debug_name = "") const;
				void Destroy(vk::ShaderModule& handle);

				vk::PipelineLayout CreatePipelineLayout(const vk::PipelineLayoutCreateInfo& create_info, const std::string& debug_name = "") const;
				void Destroy(vk::PipelineLayout& handle) const;

				vk::QueryPool CreateQueryPool(const vk::QueryPoolCreateInfo& create_info, const std::string& debug_name = "") const;
				vk::Result GetQueryResult(vk::QueryPool& handle, uint32_t first_query, uint32_t query_count, vk::DeviceSize stride, vk::QueryResultFlags flags) const;
				vk::Result GetQueryResults(vk::QueryPool& handle, uint32_t first_query, uint32_t query_count, vk::DeviceSize data_size, void* data_ptr, vk::DeviceSize stride, vk::QueryResultFlags flags) const;
				void ResetQueryPool(vk::QueryPool& handle, uint32_t first_query, uint32_t query_count) const;
				void Destroy(vk::QueryPool& handle) const;

				void Wait();

				void GetOccupedDeviceMemory(vk::MemoryHeapFlags memory_flags, u64& total_memory, u64& used_memory);

				void SubmitCommandBuffer(vk::CommandBuffer& command_buffer);

				void InitProfiler(vk::CommandBuffer& command_pool);

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
			protected:
				void RegisterAllocationCallbacks();
				void EnableDeviceFeatures(PhysicalDevice& device);
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

				/**
				 * @brief A list of surface formats in order of priority (vector[0] has high priority, vector[size-1] has low priority)
				 */
				std::vector<vk::SurfaceFormatKHR> surface_priority_list = {
					{vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear},
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