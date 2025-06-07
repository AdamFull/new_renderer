#include "D3D12Context.h"

namespace edge::gfx::d3d12
{
	D3D12Context::D3D12Context(const ContextCreateInfo& create_info) {
		assert(false && "NOT IMPLEMENTED");
	}

	D3D12Context::~D3D12Context() {
	}

	void D3D12Context::destroy(IGFXResource* handle) const {
		assert(false && "NOT IMPLEMENTED");
	}

	auto D3D12Context::create_fence() const -> IFence* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_semaphore(SemaphoreType type) const -> ISemaphore* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::get_queue_count(QueueType queue_type) const->uint32_t {
		assert(false && "NOT IMPLEMENTED");
		return 0;
	}

	auto D3D12Context::get_queue(QueueType queue_type, uint32_t queue_index) const->IQueue const* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_swapchain(const SwapChainCreateInfo& create_info) const -> ISwapchain* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_command_pool(const IQueue& queue, CommandPoolResetMode reset_mode) const -> ICommandPool* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_buffer(const BufferCreateInfo& create_info) const -> IBuffer* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_texture(const TextureCreateInfo& create_info) const -> ITexture* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_swapchain_texture(const ISwapchain& swapchain, uint32_t image_index) const -> ITexture* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_shader(std::span<const ShaderStageInfo> shader_stages, ShaderReflection* refl) const -> IShader* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_root_signature(const RootSignatureCreateInfo& create_info) const -> IRootSignature* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_pipeline(const GraphicsPipelineCreateInfo& create_info) const -> IPipeline* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_pipeline(const ComputePipelineCreateInfo& create_info) const -> IPipeline*{
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_pipeline_cache(const PipelineCacheCreateInfo& create_info) const -> IPipelineCache* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_descriptor_allocator() const -> IDescriptorAllocator* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_descriptor_layout(const DescriptorLayoutCreateInfo& create_info) const -> IDescriptorLayout* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_binding_layout(const IDescriptorLayout& descriptor_layout) const->IBindingLayout* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_sampler(const SamplerCreateInfo& create_info) const -> ISampler* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_query_pool(const QueryPoolCreateInfo& create_info) const -> IQueryPool* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	auto D3D12Context::create_acceleration_structure(const AccelerationStructureCreateInfo& create_info) const -> IAccelerationStructure* {
		assert(false && "NOT IMPLEMENTED");
		return nullptr;
	}

	void D3D12Context::wait_for_fences(std::span<IFence*> fences, uint64_t timeout) const {
		assert(false && "NOT IMPLEMENTED");
	}

	void D3D12Context::request_minimal_buffer_alignment(BufferUsageFlags usage, uint64_t& alignment) const {
		assert(false && "NOT IMPLEMENTED");
	}

	void D3D12Context::request_timestamp_frequency(double* frequency) const {
		assert(false && "NOT IMPLEMENTED");
	}

	void D3D12Context::request_memory_stats(MemoryUsageStats& memory_stats) const {
		assert(false && "NOT IMPLEMENTED");
	}
}