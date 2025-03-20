#pragma once

#include <memory>
#include "gfx_base.h"

#ifdef create_semaphore
#undef create_semaphore
#endif

namespace edge::gfx
{
	// TODO: may be return pointers here and add release function also
	class IContext {
	public:
		virtual ~IContext() = default;

		virtual void destroy(IGFXResource* handle) const = 0;

		[[nodiscard]] virtual auto create_fence() const->IFence* = 0;

		[[nodiscard]] virtual auto create_timeline_semaphore() const->ISemaphore* = 0;
		[[nodiscard]] virtual auto create_semaphore() const->ISemaphore* = 0;

		[[nodiscard]] virtual auto request_queue(QueueTypeFlags include_queue, QueueTypeFlags exclude_queue = {}) const->IQueue const* = 0;

		[[nodiscard]] virtual auto create_swapchain(const SwapChainCreateInfo& create_info) const->ISwapchain* = 0;

		[[nodiscard]] virtual auto create_command_pool(const IQueue* queue, CommandPoolResetMode reset_mode = CommandPoolResetMode::eAlwaysAllocate) const->ICommandPool* = 0;

		[[nodiscard]] virtual auto create_buffer(const BufferCreateInfo& create_info) const->IBuffer* = 0;

		[[nodiscard]] virtual auto create_texture(const TextureCreateInfo& create_info) const->ITexture* = 0;
		[[nodiscard]] virtual auto create_swapchain_texture(ISwapchain* swapchain, uint32_t image_index) const->ITexture* = 0;

		[[nodiscard]] virtual auto create_shader(std::span<const ShaderStageInfo> shader_stages) const->IShader* = 0;

		[[nodiscard]] virtual auto create_root_signature(const RootSignatureCreateInfo& create_info) const->IRootSignature* = 0;

		[[nodiscard]] virtual auto create_pipeline(const GraphicsPipelineCreateInfo& create_info) const->IPipeline* = 0;
		[[nodiscard]] virtual auto create_pipeline(const ComputePipelineCreateInfo& create_info) const->IPipeline* = 0;

		[[nodiscard]] virtual auto create_pipeline_cache(const PipelineCacheCreateInfo& create_info) const->IPipelineCache* = 0;

		[[nodiscard]] virtual auto create_descriptor_allocator() const->IDescriptorAllocator* = 0;

		[[nodiscard]] virtual auto create_shared_descriptor(const SharedDescriptorCreateInfo& create_info) const->ISharedDescriptor* = 0;

		[[nodiscard]] virtual auto create_sampler(const SamplerCreateInfo& create_info) const->ISampler* = 0;

		[[nodiscard]] virtual auto create_query_pool(const QueryPoolCreateInfo& create_info) const->IQueryPool* = 0;

		virtual void wait_for_fences(std::span<IFence*> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const = 0;

		virtual void request_minimal_buffer_alignment(BufferUsageFlags usage, uint64_t& alignment) const = 0;
		virtual void request_timestamp_frequency(float* frequency) const = 0;
		virtual void request_memory_usage(uint64_t* total_memory, uint64_t* used_memory) const = 0;
	};

	std::unique_ptr<IContext> create_context(const ContextCreateInfo& create_info);
}