#pragma once

#include "../gfx_context.h"

namespace edge
{
	namespace gfx
	{
		namespace d3d12
		{
			class D3D12Context final : public Context
			{
			public:
				D3D12Context(const ContextCreateInfo& create_info);
				~D3D12Context();

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

				void WaitForFences(std::span<Fence*> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()) override;

				void RequestMinimalBufferAlignment(BufferUsageFlags usage, uint64_t& alignment) override;
				void RequestTimestampFrequency(float* frequency) override;
				void RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory) override;
			};
		}
	}
}