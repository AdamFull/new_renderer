#pragma once

#include <memory>
#include "gfx_base.h"

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

namespace edge
{
	namespace gfx
	{
		// TODO: may be return pointers here and add release function also
		class Context
		{
		public:
			virtual ~Context() = default;

			[[nodiscard]] virtual std::unique_ptr<Fence> CreateFence() = 0;

			[[nodiscard]] virtual std::unique_ptr<Semaphore> CreateSemaphore() = 0;

			[[nodiscard]] virtual const Queue* RequestQueue(QueueTypeFlags queue_type_flags, uint32_t queue_index = 0u) = 0;

			[[nodiscard]] virtual std::unique_ptr<Swapchain> CreateSwapchain(const SwapChainCreateInfo& create_info) = 0;

			[[nodiscard]] virtual std::unique_ptr<CommandPool> CreateCommandPool(const Queue* queue, CommandPoolResetMode reset_mode = CommandPoolResetMode::eAlwaysAllocate) = 0;

			[[nodiscard]] virtual std::unique_ptr<Buffer> CreateBuffer(const BufferCreateInfo& create_info) = 0;

			[[nodiscard]] virtual std::unique_ptr<Texture> CreateTexture(const TextureCreateInfo& create_info) = 0;
			[[nodiscard]] virtual std::unique_ptr<Texture> CreateSwapchainTexture(Swapchain* swapchain, uint32_t image_index) = 0;

			[[nodiscard]] virtual std::unique_ptr<Shader> CreateShader(std::span<const ShaderStageInfo> shader_stages) = 0;

			[[nodiscard]] virtual std::unique_ptr<RootSignature> CreateRootSignature(const RootSignatureCreateInfo& create_info) = 0;

			[[nodiscard]] virtual std::unique_ptr<Pipeline> CreatePipeline(const GraphicsPipelineCreateInfo& create_info) = 0;
			[[nodiscard]] virtual std::unique_ptr<Pipeline> CreatePipeline(const ComputePipelineCreateInfo& create_info) = 0;

			[[nodiscard]] virtual std::unique_ptr<PipelineCache> CreatePipelineCache(const PipelineCacheCreateInfo& create_info) = 0;

			[[nodiscard]] virtual std::unique_ptr<DescriptorAllocator> CreateDescriptorAllocator() = 0;

			[[nodiscard]] virtual std::unique_ptr<SharedDescriptor> CreateSharedDescriptor(const SharedDescriptorCreateInfo& create_info) = 0;

			[[nodiscard]] virtual std::unique_ptr<Sampler> CreateSampler(const SamplerCreateInfo& create_info) = 0;

			[[nodiscard]] virtual std::unique_ptr<QueryPool> CreateQueryPool(const QueryPoolCreateInfo& create_info) = 0;

			virtual void WaitForFences(std::span<Fence*> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()) = 0;

			virtual void RequestMinimalBufferAlignment(BufferUsageFlags usage, uint64_t& alignment) = 0;
			virtual void RequestTimestampFrequency(float* frequency) = 0;
			virtual void RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory) = 0;
		};

		std::unique_ptr<Context> CreateContext(const ContextCreateInfo& create_info);
	}
}