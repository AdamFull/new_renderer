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

			virtual void Destroy(GFXResource* handle) = 0;

			virtual Fence* CreateFence() = 0;

			virtual Semaphore* CreateSemaphore() = 0;

			virtual const Queue* RequestQueue(QueueTypeFlags include_queue, QueueTypeFlags exclude_queue = {}) = 0;

			virtual Swapchain* CreateSwapchain(const SwapChainCreateInfo& create_info) = 0;

			virtual CommandPool* CreateCommandPool(const Queue* queue, CommandPoolResetMode reset_mode = CommandPoolResetMode::eAlwaysAllocate) = 0;

			virtual Buffer* CreateBuffer(const BufferCreateInfo& create_info) = 0;

			virtual Texture* CreateTexture(const TextureCreateInfo& create_info) = 0;
			virtual Texture* CreateSwapchainTexture(Swapchain* swapchain, uint32_t image_index) = 0;

			virtual Shader* CreateShader(std::span<const ShaderStageInfo> shader_stages) = 0;

			virtual RootSignature* CreateRootSignature(const RootSignatureCreateInfo& create_info) = 0;

			virtual Pipeline* CreatePipeline(const GraphicsPipelineCreateInfo& create_info) = 0;
			virtual Pipeline* CreatePipeline(const ComputePipelineCreateInfo& create_info) = 0;

			virtual PipelineCache* CreatePipelineCache(const PipelineCacheCreateInfo& create_info) = 0;

			virtual DescriptorAllocator* CreateDescriptorAllocator() = 0;

			virtual SharedDescriptor* CreateSharedDescriptor(const SharedDescriptorCreateInfo& create_info) = 0;

			virtual Sampler* CreateSampler(const SamplerCreateInfo& create_info) = 0;

			virtual QueryPool* CreateQueryPool(const QueryPoolCreateInfo& create_info) = 0;

			virtual void WaitForFences(std::span<Fence*> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()) = 0;

			virtual void RequestMinimalBufferAlignment(BufferUsageFlags usage, uint64_t& alignment) = 0;
			virtual void RequestTimestampFrequency(float* frequency) = 0;
			virtual void RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory) = 0;
		};

		std::unique_ptr<Context> CreateContext(const ContextCreateInfo& create_info);
	}
}