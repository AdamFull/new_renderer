#pragma once

#include <memory>
#include "gfx_base.h"

namespace edge::gfx
{
	void make_context(const ContextCreateInfo& create_info);
	void shutdown_context();

	void destroy(IGFXResource* handle);

	[[nodiscard]] auto create_fence() -> IFence*;
	[[nodiscard]] auto create_semaphore(SemaphoreType sem_type) -> ISemaphore*;

	[[nodiscard]] auto get_queue_qount(QueueType queue_type) -> uint32_t;
	[[nodiscard]] auto get_queue(QueueType queue_type, uint32_t queue_index = 0) -> IQueue const*;

	[[nodiscard]] auto create_swapchain(const SwapChainCreateInfo& create_info) -> ISwapchain*;

	[[nodiscard]] auto create_command_pool(const IQueue* queue, CommandPoolResetMode reset_mode = CommandPoolResetMode::eAlwaysAllocate) -> ICommandPool*;

	[[nodiscard]] auto create_buffer(const BufferCreateInfo& create_info) -> IBuffer*;

	[[nodiscard]] auto create_texture(const TextureCreateInfo& create_info) -> ITexture*;
	[[nodiscard]] auto create_swapchain_texture(ISwapchain* swapchain, uint32_t image_index) -> ITexture*;

	[[nodiscard]] auto create_shader(std::span<const ShaderStageInfo> shader_stages) -> IShader*;

	[[nodiscard]] auto create_root_signature(const RootSignatureCreateInfo& create_info) -> IRootSignature*;

	[[nodiscard]] auto create_pipeline(const GraphicsPipelineCreateInfo& create_info) -> IPipeline*;
	[[nodiscard]] auto create_pipeline(const ComputePipelineCreateInfo& create_info) -> IPipeline*;

	[[nodiscard]] auto create_pipeline_cache(const PipelineCacheCreateInfo& create_info) -> IPipelineCache*;

	[[nodiscard]] auto create_descriptor_allocator() -> IDescriptorAllocator*;
	[[nodiscard]] auto create_descriptor_layout(const DescriptorLayoutCreateInfo& create_info) -> IDescriptorLayout*;
	[[nodiscard]] auto create_binding_layout(IDescriptorLayout* descriptor_layout) -> IBindingLayout*;

	[[nodiscard]] auto create_sampler(const SamplerCreateInfo& create_info) -> ISampler*;

	[[nodiscard]] auto create_query_pool(const QueryPoolCreateInfo& create_info) -> IQueryPool*;

	void wait_for_fences(std::span<IFence*> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max());

	void request_minimal_buffer_alignment(BufferUsageFlags usage, uint64_t& alignment);
	void request_timestamp_frequency(float* frequency);
	void request_memory_usage(uint64_t* total_memory, uint64_t* used_memory);

	// TODO: may be return pointers here and add release function also
	class IGFXContext {
	public:
		virtual ~IGFXContext() = default;

		virtual void destroy(IGFXResource* handle) const = 0;

		virtual auto create_fence() const->IFence* = 0;

		virtual auto create_semaphore(SemaphoreType type) const->ISemaphore* = 0;

		virtual auto get_queue_count(QueueType queue_type) const->uint32_t = 0;
		virtual auto get_queue(QueueType queue_type, uint32_t queue_index = 0) const->IQueue const* = 0;

		virtual auto create_swapchain(const SwapChainCreateInfo& create_info) const->ISwapchain* = 0;

		virtual auto create_command_pool(const IQueue* queue, CommandPoolResetMode reset_mode = CommandPoolResetMode::eAlwaysAllocate) const->ICommandPool* = 0;

		virtual auto create_buffer(const BufferCreateInfo& create_info) const->IBuffer* = 0;

		virtual auto create_texture(const TextureCreateInfo& create_info) const->ITexture* = 0;
		virtual auto create_swapchain_texture(ISwapchain* swapchain, uint32_t image_index) const->ITexture* = 0;

		virtual auto create_shader(std::span<const ShaderStageInfo> shader_stages) const->IShader* = 0;

		virtual auto create_root_signature(const RootSignatureCreateInfo& create_info) const->IRootSignature* = 0;

		virtual auto create_pipeline(const GraphicsPipelineCreateInfo& create_info) const->IPipeline* = 0;
		virtual auto create_pipeline(const ComputePipelineCreateInfo& create_info) const->IPipeline* = 0;

		virtual auto create_pipeline_cache(const PipelineCacheCreateInfo& create_info) const->IPipelineCache* = 0;

		virtual auto create_descriptor_allocator() const->IDescriptorAllocator* = 0;
		virtual auto create_descriptor_layout(const DescriptorLayoutCreateInfo& create_info) const->IDescriptorLayout* = 0;
		virtual auto create_binding_layout(IDescriptorLayout* descriptor_layout) const->IBindingLayout* = 0;

		virtual auto create_sampler(const SamplerCreateInfo& create_info) const->ISampler* = 0;

		virtual auto create_query_pool(const QueryPoolCreateInfo& create_info) const->IQueryPool* = 0;

		virtual void wait_for_fences(std::span<IFence*> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const = 0;

		virtual void request_minimal_buffer_alignment(BufferUsageFlags usage, uint64_t& alignment) const = 0;
		virtual void request_timestamp_frequency(float* frequency) const = 0;
		virtual void request_memory_usage(uint64_t* total_memory, uint64_t* used_memory) const = 0;
	};
}