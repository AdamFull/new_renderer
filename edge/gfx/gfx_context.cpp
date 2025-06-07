#include "gfx_context.h"

#if D3D12_SUPPORTED
#include "directx12/D3D12Context.h"
#endif

#if VULKAN_SUPPORTED
#include "vulkan/gfx_vk_context.h"
#endif

#include "../util/Logger.h"
#include "../util/utl_enum.hpp"

#define CHECK_CONTEXT_R() { if (!gfx_context) { EDGE_LOGE("[GFX] Graphics context is not initialized yet."); return; }}
#define CHECK_CONTEXT() { if (!gfx_context) { EDGE_LOGE("[GFX] Graphics context is not initialized yet."); return {}; }}

namespace edge::gfx
{
	static std::unique_ptr<IGFXContext> gfx_context;

	template<typename _Ty>
	auto make_unique(_Ty* ptr) -> std::unique_ptr<_Ty> {
		std::unique_ptr<_Ty> uptr;
		uptr.reset(ptr);
		return uptr;
	}

	void make_context(const ContextCreateInfo& create_info) {
		switch (create_info.render_api) {
#if D3D12_SUPPORTED
		case RenderApi::eDirectx12:
			gfx_context = std::make_unique<d3d12::D3D12Context>(create_info);
			break;
#endif
#if VULKAN_SUPPORTED
		case RenderApi::eVulkan:
			gfx_context = std::make_unique<vulkan::GFXContext>(create_info);
			break;
#endif
		default:
			break;
		}
	}

	void shutdown_context() {
		gfx_context.reset();
	}

	void destroy(IGFXResource* handle) {
		CHECK_CONTEXT_R();
		gfx_context->destroy(handle);
	}

	auto create_fence() -> IFence* {
		CHECK_CONTEXT();
        return gfx_context->create_fence();
	}

	auto create_fence_unique() -> std::unique_ptr<IFence> {
		return make_unique(create_fence());
	}

	auto create_semaphore(SemaphoreType sem_type) -> ISemaphore* {
		CHECK_CONTEXT();
        return gfx_context->create_semaphore(sem_type);
	}

	auto create_semaphore_unique(SemaphoreType sem_type) -> std::unique_ptr<ISemaphore> {
		return make_unique(create_semaphore(sem_type));
	}

	auto get_queue_qount(QueueType queue_type) -> uint32_t {
		CHECK_CONTEXT();
		return gfx_context->get_queue_count(queue_type);
	}

	auto get_queue(QueueType queue_type, uint32_t queue_index) -> IQueue const* {
		CHECK_CONTEXT();
		return gfx_context->get_queue(queue_type, queue_index);
	}

	auto create_swapchain(const SwapChainCreateInfo& create_info) -> ISwapchain* {
		CHECK_CONTEXT();
        return gfx_context->create_swapchain(create_info);
	}

	auto create_swapchain_unique(const SwapChainCreateInfo& create_info) -> std::unique_ptr<ISwapchain> {
		return make_unique(create_swapchain(create_info));
	}

	auto create_command_pool(const IQueue& queue, CommandPoolResetMode reset_mode) -> ICommandPool* {
		CHECK_CONTEXT();
        return gfx_context->create_command_pool(queue, reset_mode);
	}

	auto create_command_pool_unique(const IQueue& queue, CommandPoolResetMode reset_mode) -> std::unique_ptr<ICommandPool> {
		return make_unique(create_command_pool(queue, reset_mode));
	}

	auto create_buffer(const BufferCreateInfo& create_info) -> IBuffer* {
		CHECK_CONTEXT();
        return gfx_context->create_buffer(create_info);
	}

	auto create_buffer_unique(const BufferCreateInfo& create_info) -> std::unique_ptr<IBuffer> {
		return make_unique(create_buffer(create_info));
	}

	auto create_texture(const TextureCreateInfo& create_info) -> ITexture* {
		CHECK_CONTEXT();
        return gfx_context->create_texture(create_info);
	}

	auto create_texture_unique(const TextureCreateInfo& create_info) -> std::unique_ptr<ITexture> {
		return make_unique(create_texture(create_info));
	}

	auto create_swapchain_texture(const ISwapchain& swapchain, uint32_t image_index) -> ITexture* {
		CHECK_CONTEXT();
        return gfx_context->create_swapchain_texture(swapchain, image_index);
	}

	auto create_swapchain_texture_unique(const ISwapchain& swapchain, uint32_t image_index) -> std::unique_ptr<ITexture> {
		return make_unique(create_swapchain_texture(swapchain, image_index));
	}

	auto create_shader(std::span<const ShaderStageInfo> shader_stages, ShaderReflection* refl) -> IShader* {
		CHECK_CONTEXT();
        return gfx_context->create_shader(shader_stages, refl);
	}

	auto create_shader_unique(std::span<const ShaderStageInfo> shader_stages, ShaderReflection* refl) -> std::unique_ptr<IShader> {
		return make_unique(create_shader(shader_stages, refl));
	}

	auto create_root_signature(const RootSignatureCreateInfo& create_info) -> IRootSignature* {
		CHECK_CONTEXT();
        return gfx_context->create_root_signature(create_info);
	}

	auto create_root_signature_unique(const RootSignatureCreateInfo& create_info) -> std::unique_ptr<IRootSignature> {
		return make_unique(create_root_signature(create_info));
	}

	auto create_pipeline(const GraphicsPipelineCreateInfo& create_info) -> IPipeline* {
		CHECK_CONTEXT();
        return gfx_context->create_pipeline(create_info);
	}

	auto create_pipeline_unique(const GraphicsPipelineCreateInfo& create_info) -> std::unique_ptr<IPipeline> {
		return make_unique(create_pipeline(create_info));
	}

	auto create_pipeline(const ComputePipelineCreateInfo& create_info) -> IPipeline* {
		CHECK_CONTEXT();
        return gfx_context->create_pipeline(create_info);
	}

	auto create_pipeline_unique(const ComputePipelineCreateInfo& create_info) -> std::unique_ptr<IPipeline> {
		return make_unique(create_pipeline(create_info));
	}

	auto create_pipeline_cache(const PipelineCacheCreateInfo& create_info) -> IPipelineCache* {
		CHECK_CONTEXT();
        return gfx_context->create_pipeline_cache(create_info);
	}

	auto create_pipeline_cache_unique(const PipelineCacheCreateInfo& create_info) -> std::unique_ptr<IPipelineCache> {
		return make_unique(create_pipeline_cache(create_info));
	}

	auto create_descriptor_allocator() -> IDescriptorAllocator* {
		CHECK_CONTEXT();
        return gfx_context->create_descriptor_allocator();
	}

	auto create_descriptor_allocator_unique() -> std::unique_ptr<IDescriptorAllocator> {
		return make_unique(create_descriptor_allocator());
	}

	auto create_descriptor_layout(const DescriptorLayoutCreateInfo& create_info) -> IDescriptorLayout* {
		CHECK_CONTEXT();
		return gfx_context->create_descriptor_layout(create_info);
	}

	auto create_descriptor_layout_unique(const DescriptorLayoutCreateInfo& create_info) -> std::unique_ptr<IDescriptorLayout> {
		return make_unique(create_descriptor_layout(create_info));
	}

	auto create_binding_layout(const IDescriptorLayout& descriptor_layout) -> IBindingLayout* {
		CHECK_CONTEXT();
		return gfx_context->create_binding_layout(descriptor_layout);
	}

	auto create_binding_layout_unique(const IDescriptorLayout& descriptor_layout) -> std::unique_ptr<IBindingLayout> {
		return make_unique(create_binding_layout(descriptor_layout));
	}

	auto create_sampler(const SamplerCreateInfo& create_info) -> ISampler* {
		CHECK_CONTEXT();
        return gfx_context->create_sampler(create_info);
	}

	auto create_sampler_unique(const SamplerCreateInfo& create_info) -> std::unique_ptr<ISampler> {
		return make_unique(create_sampler(create_info));
	}

	auto create_query_pool(const QueryPoolCreateInfo& create_info) -> IQueryPool* {
		CHECK_CONTEXT();
        return gfx_context->create_query_pool(create_info);
	}

	auto create_query_pool_unique(const QueryPoolCreateInfo& create_info) -> std::unique_ptr<IQueryPool> {
		return make_unique(create_query_pool(create_info));
	}

	void wait_for_fences(std::span<IFence*> fences, uint64_t timeout) {
		CHECK_CONTEXT_R();
		gfx_context->wait_for_fences(fences, timeout);
	}

	void request_minimal_buffer_alignment(BufferUsageFlags usage, uint64_t& alignment) {
		CHECK_CONTEXT_R();
		gfx_context->request_minimal_buffer_alignment(usage, alignment);
	}

	void request_timestamp_frequency(double* frequency) {
		CHECK_CONTEXT_R();
		gfx_context->request_timestamp_frequency(frequency);
	}

	void request_memory_stats(MemoryUsageStats& memory_stats) {
		CHECK_CONTEXT_R();
		gfx_context->request_memory_stats(memory_stats);
	}
}