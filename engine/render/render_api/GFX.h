#pragma once

#include <memory>
#include <GFXStructures.h>

namespace gfx
{
	class GFXResource
	{
	public:
		virtual ~GFXResource() = default;

	protected:
		ResourceType resource_type{};
	};

	class Fence : public GFXResource
	{
	public:
		virtual ~Fence() = default;

		virtual void Wait() = 0;
	};

	class Semaphore : public GFXResource
	{
	public:
		virtual ~Semaphore() = default;
	};

	class Queue : public GFXResource
	{
	public:
		virtual ~Queue() = default;

		virtual void Submit() = 0;
		virtual void Present() = 0;
		virtual void WaitIdle() = 0;
	};

	class Swapchain : public GFXResource
	{
	public:
		virtual ~Swapchain() = default;

		virtual void AcquireNextImage(Semaphore* signal_semaphore, Fence* fence, uint32_t* image_index) = 0;
	};

	class Buffer : public GFXResource
	{
	public:
		virtual ~Buffer() = default;
	};

	class CommandList : public GFXResource
	{
	public:
		virtual ~CommandList() = default;

		virtual void Reset() = 0;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void SetViewport() = 0;
		virtual void SerScissor() = 0;

		virtual void BindPipeline() = 0;
		virtual void BindDescriptorSet() = 0;
		virtual void PushConstants() = 0;

		virtual void BindIndexBuffer() = 0;
		virtual void BindVertexBuffers() = 0;

		virtual void Draw(uint32_t vertex_count, uint32_t first_vertex, uint32_t first_instance, uint32_t instance_count) = 0;
		virtual void DrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) = 0;
		virtual void DrawIndirect() = 0;
		virtual void DrawIndirectIndexed() = 0;
		virtual void DrawIndirectIndexedCount() = 0;

		virtual void WriteTimestamp() = 0;

		virtual void Dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) = 0;
		virtual void DispatchIndirect() = 0;

		virtual void PipelineBarrier() = 0;

		virtual void BeginQuery(QueryPool* query_pool) = 0;
		virtual void EndQuery(QueryPool* query_pool) = 0;

		virtual void BeginRendering() = 0;
		virtual void EndRendering() = 0;

		virtual void BeginMarker(const char* name, uint32_t color) = 0;
		virtual void InsertMarker(const char* name, uint32_t color) = 0;
		virtual void EndMarker() = 0;
	};

	class CommandPool : public GFXResource
	{
	public:
		virtual ~CommandPool() = default;

		virtual CommandList* Allocate(bool secondary = false) = 0;
		virtual void Reset() = 0;
	};

	class Texture : public GFXResource
	{
	public:
		virtual ~Texture() = default;
	};

	class RenderTarget : public GFXResource
	{
	public:
		virtual ~RenderTarget() = default;
	};

	class Sampler : public GFXResource
	{
	public:
		virtual ~Sampler() = default;
	};

	class Shader : public GFXResource
	{
	public:
		virtual ~Shader() = default;
	};

	class RootSignature : public GFXResource
	{
	public:
		virtual ~RootSignature() = default;
	};

	class Pipeline : public GFXResource
	{
	public:
		virtual ~Pipeline() = default;
	};

	class PipelineCache : public GFXResource
	{
	public:
		virtual ~PipelineCache() = default;

		virtual void GetData(size_t& data_size, void* data_ptr) = 0;
	};

	class DescriptorSet : public GFXResource
	{
	public:
		virtual ~DescriptorSet() = default;

		virtual void Reset() = 0;
		virtual void Update() = 0;
	};

	class QueryPool : public GFXResource
	{
	public:
		virtual ~QueryPool() = default;

		virtual void Reset(uint32_t start_query, uint32_t query_count) = 0;
		virtual void GetData(uint32_t query_index, void* data) = 0;
	};

	// TODO: may be return pointers here and add release function also
	class GFXContext
	{
	public:
		virtual ~GFXContext() = default;

		virtual Fence* CreateFence() = 0;
		virtual void DestroyFence(Fence* fence) = 0;

		virtual Semaphore* CreateSemaphore() = 0;
		virtual void DestroyFence(Semaphore* semaphore) = 0;

		virtual Queue* RequestQueue(QueueTypeFlags queue_type_flags, uint32_t queue_index = 0u) = 0;

		virtual Swapchain* CreateSwapchain(const SwapChainCreateInfo& create_info) = 0;
		virtual void DestroySwapchain(Swapchain* swapchain) = 0;

		virtual CommandPool* CreateCommandPool(Queue* queue) = 0;
		virtual void DestroyCommandPool(CommandPool* command_pool) = 0;

		virtual Buffer* CreateBuffer(const BufferCreateInfo& create_info) = 0;
		virtual void DestroyBuffer(Buffer* buffer) = 0;

		virtual Texture* CreateTexture(const TextureCreateInfo& create_info) = 0;
		virtual void DestroyTexture(Texture* texture) = 0;

		virtual RenderTarget* CreateRenderTarget(const RenderTargetCreateInfo& create_info) = 0;
		virtual void DestroyRenderTarget(RenderTarget* render_target) = 0;

		virtual Sampler* CreateSampler(const SamplerCreateInfo& create_info) = 0;
		virtual void DestroySampler(Sampler* sampler) = 0;

		virtual QueryPool* CreateQueryPool() = 0;
		virtual void DestroyQueryPool(QueryPool* query_pool) = 0;

		virtual void RequestTimestampFrequency(double* frequency) = 0;
		virtual void RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory) = 0;
		
		virtual void SetResourceName(GFXResource* resource_handle, const char* name) = 0;
	};

	std::unique_ptr<GFXContext> CreateGFXContext(const char* app_name, RenderApi render_api = RenderApi::eAuto);
}