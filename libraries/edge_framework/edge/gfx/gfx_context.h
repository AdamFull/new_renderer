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
		class IContext
		{
		public:
			virtual ~IContext() = default;

			virtual void Destroy(IGFXResource* handle) const = 0;

			[[nodiscard]] virtual auto CreateFence() const -> IFence* = 0;

			[[nodiscard]] virtual auto CreateTimelineSemaphore() const -> ISemaphore* = 0;
			[[nodiscard]] virtual auto CreateSemaphore() const -> ISemaphore* = 0;

			[[nodiscard]] virtual auto RequestQueue(QueueTypeFlags include_queue, QueueTypeFlags exclude_queue = {}) const -> IQueue const* = 0;

			[[nodiscard]] virtual auto CreateSwapchain(const SwapChainCreateInfo& create_info) const -> ISwapchain* = 0;

			[[nodiscard]] virtual auto CreateCommandPool(const IQueue* queue, CommandPoolResetMode reset_mode = CommandPoolResetMode::eAlwaysAllocate) const -> ICommandPool* = 0;

			[[nodiscard]] virtual auto CreateBuffer(const BufferCreateInfo& create_info) const -> IBuffer* = 0;

			[[nodiscard]] virtual auto CreateTexture(const TextureCreateInfo& create_info) const -> ITexture* = 0;
			[[nodiscard]] virtual auto CreateSwapchainTexture(ISwapchain* swapchain, uint32_t image_index) const -> ITexture* = 0;

			[[nodiscard]] virtual auto CreateShader(std::span<const ShaderStageInfo> shader_stages) const -> IShader* = 0;

			[[nodiscard]] virtual auto CreateRootSignature(const RootSignatureCreateInfo& create_info) const -> IRootSignature* = 0;

			[[nodiscard]] virtual auto CreatePipeline(const GraphicsPipelineCreateInfo& create_info) const -> IPipeline* = 0;
			[[nodiscard]] virtual auto CreatePipeline(const ComputePipelineCreateInfo& create_info) const -> IPipeline* = 0;

			[[nodiscard]] virtual auto CreatePipelineCache(const PipelineCacheCreateInfo& create_info) const -> IPipelineCache* = 0;

			[[nodiscard]] virtual auto CreateDescriptorAllocator() const -> IDescriptorAllocator* = 0;

			[[nodiscard]] virtual auto CreateSharedDescriptor(const SharedDescriptorCreateInfo& create_info) const -> ISharedDescriptor* = 0;

			[[nodiscard]] virtual auto CreateSampler(const SamplerCreateInfo& create_info) const ->ISampler* = 0;

			[[nodiscard]] virtual auto CreateQueryPool(const QueryPoolCreateInfo& create_info) const -> IQueryPool* = 0;

			virtual void WaitForFences(std::span<IFence*> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const = 0;

			virtual void RequestMinimalBufferAlignment(BufferUsageFlags usage, uint64_t& alignment) const = 0;
			virtual void RequestTimestampFrequency(float* frequency) const = 0;
			virtual void RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory) const = 0;
		};

		std::unique_ptr<IContext> CreateContext(const ContextCreateInfo& create_info);
	}
}