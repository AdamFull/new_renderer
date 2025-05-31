#pragma once

#include "../gfx_context.h"

namespace edge::gfx::d3d12
{
	class D3D12Context final : public IGFXContext
	{
	public:
		D3D12Context(const ContextCreateInfo& create_info);
		~D3D12Context();

		void destroy(IGFXResource* handle) const override;

		auto create_fence() const->IFence* override;

		auto create_semaphore(SemaphoreType type) const->ISemaphore* override;

		auto get_queue_count(QueueType queue_type) const->uint32_t override;
		auto get_queue(QueueType queue_type, uint32_t queue_index = 0) const->IQueue const* override;

		auto create_swapchain(const SwapChainCreateInfo& create_info) const->ISwapchain* override;

		auto create_command_pool(const IQueue* queue, CommandPoolResetMode reset_mode) const->ICommandPool* override;

		auto create_buffer(const BufferCreateInfo& create_info) const->IBuffer* override;

		auto create_texture(const TextureCreateInfo& create_info) const->ITexture* override;
		auto create_swapchain_texture(ISwapchain* swapchain, uint32_t image_index) const->ITexture* override;

		auto create_shader(std::span<const ShaderStageInfo> shader_stages, ShaderReflection* refl = nullptr) const->IShader* override;

		auto create_root_signature(const RootSignatureCreateInfo& create_info) const->IRootSignature* override;

		auto create_pipeline(const GraphicsPipelineCreateInfo& create_info) const->IPipeline* override;
		auto create_pipeline(const ComputePipelineCreateInfo& create_info) const->IPipeline* override;

		auto create_pipeline_cache(const PipelineCacheCreateInfo& create_info) const->IPipelineCache* override;

		auto create_descriptor_allocator() const->IDescriptorAllocator* override;
		auto create_descriptor_layout(const DescriptorLayoutCreateInfo& create_info) const->IDescriptorLayout* override;
		auto create_binding_layout(IDescriptorLayout* descriptor_layout) const->IBindingLayout* override;

		auto create_sampler(const SamplerCreateInfo& create_info) const->ISampler* override;

		auto create_query_pool(const QueryPoolCreateInfo& create_info) const->IQueryPool* override;

		auto create_acceleration_structure(const AccelerationStructureCreateInfo& create_info) const->IAccelerationStructure* override;

		void wait_for_fences(std::span<IFence*> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const override;

		void request_minimal_buffer_alignment(BufferUsageFlags usage, uint64_t& alignment) const override;
		void request_timestamp_frequency(double* frequency) const override;
		void request_memory_stats(MemoryUsageStats& memory_stats) const override;
	};
}