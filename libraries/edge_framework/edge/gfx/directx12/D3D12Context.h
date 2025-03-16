#pragma once

#include "../gfx_context.h"

namespace edge
{
	namespace gfx
	{
		namespace d3d12
		{
			class D3D12Context final : public IContext
			{
			public:
				D3D12Context(const ContextCreateInfo& create_info);
				~D3D12Context();

				void Destroy(IGFXResource* handle) const override;

				[[nodiscard]] auto CreateFence() const->IFence* override;

				[[nodiscard]] auto CreateTimelineSemaphore() const->ISemaphore* override;
				[[nodiscard]] auto CreateSemaphore() const->ISemaphore* override;

				[[nodiscard]] auto RequestQueue(QueueTypeFlags include_queue, QueueTypeFlags exclude_queue = {}) const-> IQueue* override;

				[[nodiscard]] auto CreateSwapchain(const SwapChainCreateInfo& create_info) const->ISwapchain* override;

				[[nodiscard]] auto CreateCommandPool(const IQueue* queue, CommandPoolResetMode reset_mode) const->ICommandPool* override;

				[[nodiscard]] auto CreateBuffer(const BufferCreateInfo& create_info) const-> IBuffer* override;

				[[nodiscard]] auto CreateTexture(const TextureCreateInfo& create_info) const-> ITexture* override;
				[[nodiscard]] auto CreateSwapchainTexture(ISwapchain* swapchain, uint32_t image_index) const-> ITexture* override;

				[[nodiscard]] auto CreateShader(std::span<const ShaderStageInfo> shader_stages) const-> IShader* override;

				[[nodiscard]] auto CreateRootSignature(const RootSignatureCreateInfo& create_info) const->IRootSignature* override;

				[[nodiscard]] auto CreatePipeline(const GraphicsPipelineCreateInfo& create_info) const->IPipeline* override;
				[[nodiscard]] auto CreatePipeline(const ComputePipelineCreateInfo& create_info) const->IPipeline* override;

				[[nodiscard]] auto CreatePipelineCache(const PipelineCacheCreateInfo& create_info) const->IPipelineCache* override;

				[[nodiscard]] auto CreateDescriptorAllocator() const->IDescriptorAllocator* override;

				[[nodiscard]] auto CreateSharedDescriptor(const SharedDescriptorCreateInfo& create_info) const->ISharedDescriptor* override;

				[[nodiscard]] auto CreateSampler(const SamplerCreateInfo& create_info) const->ISampler* override;

				[[nodiscard]] auto CreateQueryPool(const QueryPoolCreateInfo& create_info) const->IQueryPool* override;

				void WaitForFences(std::span<IFence*> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const override;

				void RequestMinimalBufferAlignment(BufferUsageFlags usage, uint64_t& alignment) const override;
				void RequestTimestampFrequency(float* frequency) const override;
				void RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory) const override;
			};
		}
	}
}