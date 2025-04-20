#include "gfx_context.h"

#if D3D12_SUPPORTED
#include "directx12/D3D12Context.h"
#endif

#if VULKAN_SUPPORTED
#include "vulkan/gfx_vk_context.h"
#endif

#include "../util/Logger.h"
#include "../util/utl_enum.hpp"

namespace edge::gfx
{
	static std::unique_ptr<IGFXContext> gfx_context;

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
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return;
		}

		gfx_context->destroy(handle);
	}

	auto create_fence() -> IFence* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_fence();
	}

	auto create_semaphore(SemaphoreType sem_type) -> ISemaphore* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_semaphore(sem_type);
	}

	auto get_queue_qount(QueueType queue_type) -> uint32_t {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return 0;
		}

		return gfx_context->get_queue_count(queue_type);
	}

	auto get_queue(QueueType queue_type, uint32_t queue_index) -> IQueue const* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

		return gfx_context->get_queue(queue_type, queue_index);
	}

	auto create_swapchain(const SwapChainCreateInfo& create_info) -> ISwapchain* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_swapchain(create_info);
	}

	auto create_command_pool(const IQueue* queue, CommandPoolResetMode reset_mode) -> ICommandPool* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_command_pool(queue, reset_mode);
	}

	auto create_buffer(const BufferCreateInfo& create_info) -> IBuffer* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_buffer(create_info);
	}

	auto create_texture(const TextureCreateInfo& create_info) -> ITexture* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_texture(create_info);
	}

	auto create_swapchain_texture(ISwapchain* swapchain, uint32_t image_index) -> ITexture* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_swapchain_texture(swapchain, image_index);
	}

	auto create_shader(std::span<const ShaderStageInfo> shader_stages) -> IShader* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_shader(shader_stages);
	}

	auto create_root_signature(const RootSignatureCreateInfo& create_info) -> IRootSignature* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_root_signature(create_info);
	}

	auto create_pipeline(const GraphicsPipelineCreateInfo& create_info) -> IPipeline* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_pipeline(create_info);
	}

	auto create_pipeline(const ComputePipelineCreateInfo& create_info) -> IPipeline* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_pipeline(create_info);
	}

	auto create_pipeline_cache(const PipelineCacheCreateInfo& create_info) -> IPipelineCache* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_pipeline_cache(create_info);
	}

	auto create_descriptor_allocator() -> IDescriptorAllocator* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_descriptor_allocator();
	}

	auto create_descriptor_layout(const DescriptorLayoutCreateInfo& create_info) -> IDescriptorLayout* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

		return gfx_context->create_descriptor_layout(create_info);
	}

	auto create_binding_layout(IDescriptorLayout* descriptor_layout) -> IBindingLayout* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

		return gfx_context->create_binding_layout(descriptor_layout);
	}

	auto create_sampler(const SamplerCreateInfo& create_info) -> ISampler* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_sampler(create_info);
	}

	auto create_query_pool(const QueryPoolCreateInfo& create_info) -> IQueryPool* {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return nullptr;
		}

        return gfx_context->create_query_pool(create_info);
	}

	void wait_for_fences(std::span<IFence*> fences, uint64_t timeout) {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return;
		}

		gfx_context->wait_for_fences(fences, timeout);
	}

	void request_minimal_buffer_alignment(BufferUsageFlags usage, uint64_t& alignment) {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return;
		}

		gfx_context->request_minimal_buffer_alignment(usage, alignment);
	}

	void request_timestamp_frequency(float* frequency) {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return;
		}

		gfx_context->request_timestamp_frequency(frequency);
	}

	void request_memory_usage(uint64_t* total_memory, uint64_t* used_memory) {
		if (!gfx_context) {
			EDGE_LOGE("[GFX] Graphics context is not initialized yet.");
			return;
		}

		gfx_context->request_memory_usage(total_memory, used_memory);
	}
}