#pragma once

#include "../gfx_context.h"

#include <vulkan/vulkan.hpp>

#if USE_NSIGHT_AFTERMATH
#include "../gfx_nsight_aftermath.h"
#endif

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

namespace edge
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

			class VulkanContext final : public IContext
			{
			public:
				VulkanContext(const ContextCreateInfo& create_info);
				~VulkanContext();

				void Destroy(IGFXResource* handle) const override;

				[[nodiscard]] auto CreateFence() const -> IFence* override;

				[[nodiscard]] auto CreateTimelineSemaphore() const -> ISemaphore* override;
				[[nodiscard]] auto CreateSemaphore() const -> ISemaphore* override;

				[[nodiscard]] auto RequestQueue(QueueTypeFlags include_queue, QueueTypeFlags exclude_queue = {}) const -> IQueue* override;

				[[nodiscard]] auto CreateSwapchain(const SwapChainCreateInfo& create_info) const -> ISwapchain* override;

				[[nodiscard]] auto CreateCommandPool(const IQueue* queue, CommandPoolResetMode reset_mode) const -> ICommandPool* override;

				[[nodiscard]] auto CreateBuffer(const BufferCreateInfo& create_info) const -> IBuffer* override;

				[[nodiscard]] auto CreateTexture(const TextureCreateInfo& create_info) const -> ITexture* override;
				[[nodiscard]] auto CreateSwapchainTexture(ISwapchain* swapchain, uint32_t image_index) const -> ITexture* override;

				[[nodiscard]] auto CreateShader(std::span<const ShaderStageInfo> shader_stages) const -> IShader* override;

				[[nodiscard]] auto CreateRootSignature(const RootSignatureCreateInfo& create_info) const -> IRootSignature* override;

				[[nodiscard]] auto CreatePipeline(const GraphicsPipelineCreateInfo& create_info) const -> IPipeline* override;
				[[nodiscard]] auto CreatePipeline(const ComputePipelineCreateInfo& create_info) const -> IPipeline* override;

				[[nodiscard]] auto CreatePipelineCache(const PipelineCacheCreateInfo& create_info) const -> IPipelineCache* override;

				[[nodiscard]] auto CreateDescriptorAllocator() const -> IDescriptorAllocator* override;

				[[nodiscard]] auto CreateSharedDescriptor(const SharedDescriptorCreateInfo& create_info) const -> ISharedDescriptor* override;

				[[nodiscard]] auto CreateSampler(const SamplerCreateInfo& create_info) const -> ISampler* override;

				[[nodiscard]] auto CreateQueryPool(const QueryPoolCreateInfo& create_info) const -> IQueryPool* override;

				void WaitForFences(std::span<IFence*> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const override;

				void RequestMinimalBufferAlignment(BufferUsageFlags usage, uint64_t& alignment) const override;
				void RequestTimestampFrequency(float* frequency) const override;
				void RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory) const override;
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

#if USE_NSIGHT_AFTERMATH
				std::unique_ptr<NsightAftermath> nsight_aftermath;
#endif

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