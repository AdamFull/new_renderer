#pragma once

#include <VkBuffer.h>
#include <VkFence.h>
#include <VkSemaphore.h>
#include <VkSampler.h>
#include <VkPipelineCache.h>
#include <VkCommandPool.h>
#include <VkQueryPool.h>
#include <VkTexture.h>

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

namespace gfx
{
	namespace vulkan
	{
		class Instance;
		class PhysicalDevice;
		class DebugUtils;
		class Device;
		class Allocator;
		class Allocation;

		class VkGFXContext final : public GFXContext
		{
		public:
			VkGFXContext(const GFXContextCreateInfo& create_info);
			~VkGFXContext();

			Fence* CreateFence() override;
			void DestroyFence(Fence* fence) override;

			Semaphore* CreateSemaphore() override;
			void DestroySemaphore(Semaphore* semaphore) override;

			Queue* RequestQueue(QueueTypeFlags queue_type_flags, uint32_t queue_index = 0u) override;

			Swapchain* CreateSwapchain(const SwapChainCreateInfo& create_info) override;
			void DestroySwapchain(Swapchain* swapchain) override;

			CommandPool* CreateCommandPool(Queue* queue, CommandPoolResetMode reset_mode) override;
			void DestroyCommandPool(CommandPool* command_pool) override;

			Buffer* CreateBuffer(const BufferCreateInfo& create_info) override;
			void DestroyBuffer(Buffer* buffer) override;

			Texture* CreateTexture(const TextureCreateInfo& create_info) override;
			void DestroyTexture(Texture* texture) override;

			Shader* CreateShader(const ShaderCreateInfo& create_info) override;
			void DestroyShader(Shader* shader) override;

			RootSignature* CreateRootSignature(const RootSignatureCreateInfo& create_info) override;
			void DestroyRootSignature(RootSignature* root_signature) override;

			Pipeline* CreatePipeline(const GraphicsPipelineCreateInfo& create_info) override;
			Pipeline* CreatePipeline(const ComputePipelineCreateInfo& create_info) override;
			void DestroyPipeline(Pipeline* pipeline) override;

			PipelineCache* CreatePipelineCache(const PipelineCacheCreateInfo& create_info) override;
			void DestroyPipelineCache(PipelineCache* pipeline_cache) override;

			DescriptorPool* CreateDescriptorPool() override;
			void DestroyDescriptorPool(DescriptorPool* descriptor_pool) override;

			Sampler* CreateSampler(const SamplerCreateInfo& create_info) override;
			void DestroySampler(Sampler* sampler) override;

			QueryPool* CreateQueryPool(const QueryPoolCreateInfo& create_info) override;
			void DestroyQueryPool(QueryPool* query_pool) override;

			void WaitForFences(uint32_t fence_count, Fence** fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()) override;

			void RequestTimestampFrequency(double* frequency) override;
			void RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory) override;

			void SetResourceName(GFXResource* resource_handle, const char* name) override;
		private:
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

			/**
			 * @brief A list of surface formats in order of priority (vector[0] has high priority, vector[size-1] has low priority)
			 */
			std::vector<vk::SurfaceFormatKHR> surface_priority_list = {
				{vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear},
				{vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
				{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear} };

			std::vector<std::unique_ptr<VulkanFence>> fences{};
			std::vector<std::unique_ptr<VulkanSemaphore>> semaphores{};
			std::vector<std::unique_ptr<VulkanBuffer>> buffers{};
			std::vector<std::unique_ptr<VulkanSampler>> samplers{};
			std::vector<std::unique_ptr<VulkanPipelineCache>> pipeline_caches{};
			std::vector<std::unique_ptr<VulkanCommandPool>> command_pools{};
			std::vector<std::unique_ptr<VulkanQueryPool>> query_pools{};
			std::vector<std::unique_ptr<VulkanTexture>> textures{};
		};
	}
}