#pragma once

#include <Context.h>

#include <vulkan/vulkan.hpp>

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

namespace engine
{
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

			class VulkanContext final : public Context
			{
			public:
				VulkanContext(const ContextCreateInfo& create_info);
				~VulkanContext();

				void Destroy(GFXResource* handle) override;

				Fence* CreateFence() override;

				Semaphore* CreateSemaphore() override;

				const Queue* RequestQueue(QueueTypeFlags queue_type_flags, uint32_t queue_index = 0u) override;

				Swapchain* CreateSwapchain(const SwapChainCreateInfo& create_info) override;

				CommandPool* CreateCommandPool(const Queue* queue, CommandPoolResetMode reset_mode) override;

				Buffer* CreateBuffer(const BufferCreateInfo& create_info) override;

				Texture* CreateTexture(const TextureCreateInfo& create_info) override;
				Texture* CreateSwapchainTexture(Swapchain* swapchain, uint32_t image_index) override;

				Shader* CreateShader(const ShaderCreateInfo& create_info) override;

				RootSignature* CreateRootSignature(const RootSignatureCreateInfo& create_info) override;

				Pipeline* CreatePipeline(const GraphicsPipelineCreateInfo& create_info) override;
				Pipeline* CreatePipeline(const ComputePipelineCreateInfo& create_info) override;

				PipelineCache* CreatePipelineCache(const PipelineCacheCreateInfo& create_info) override;

				DescriptorAllocator* CreateDescriptorAllocator() override;

				SharedDescriptor* CreateSharedDescriptor(const SharedDescriptorCreateInfo& create_info) override;

				Sampler* CreateSampler(const SamplerCreateInfo& create_info) override;

				QueryPool* CreateQueryPool(const QueryPoolCreateInfo& create_info) override;

				void WaitForFences(uint32_t fence_count, Fence** fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()) override;

				void RequestMinimalBufferAlignment(BufferUsageFlags usage, uint64_t& alignment) override;
				void RequestTimestampFrequency(float* frequency) override;
				void RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory) override;
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
			};
		}
	}
}