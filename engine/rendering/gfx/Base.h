#pragma once

#include <memory>
#include <engine/rendering/gfx/Structures.h>

namespace engine
{
	namespace gfx
	{
		class GFXResource
		{
		public:
			virtual ~GFXResource() = default;

			virtual ResourceType GetResourceType() = 0;
		};

		class Fence : public GFXResource
		{
		public:
			virtual ~Fence() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eFence;
			}

			virtual void Wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) = 0;
			virtual void Reset() = 0;
		};

		class Semaphore : public GFXResource
		{
		public:
			virtual ~Semaphore() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eSemaphore;
			}
		};

		class Queue : public GFXResource
		{
		public:
			virtual ~Queue() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eQueue;
			}

			virtual void Submit(const SubmitInfo& submit_info, Fence* fence) const = 0;
			virtual void Present(const PresentInfo& present_info) const = 0;
			virtual void WaitIdle() const = 0;
		};

		class Swapchain : public GFXResource
		{
		public:
			virtual ~Swapchain() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eSwapchain;
			}

			virtual uint32_t GetImageCount() const = 0;
			virtual Format GetImageFormat() const = 0;

			virtual const uint32_t& GetWidth() const = 0;
			virtual const uint32_t& GetHeight() const = 0;

			virtual void GetSurfaceExtent(uint32_t& width, uint32_t& height) = 0;

			virtual bool AcquireNextImage(Semaphore* signal_semaphore, Fence* fence, uint32_t* image_index) = 0;
		};

		class Buffer : public GFXResource
		{
		public:
			virtual ~Buffer() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eBuffer;
			}

			virtual uint8_t* Map() = 0;
			virtual void Unmap() = 0;

			virtual uint64_t Update(const void* data, uint64_t size, uint64_t offset = 0ull) = 0;

			virtual uint64_t GetSize() const = 0;
			virtual uint64_t GetAddress() const = 0;

			virtual BufferUsageFlags GetUsageFlags() const = 0;
		};

		class BufferSuballocation final
		{
		public:
			BufferSuballocation(Buffer& buffer, uint64_t offset, uint64_t size) :
				buffer{ buffer },
				offset{ offset },
				size{ size }
			{
			}

			~BufferSuballocation() = default;

			BufferSuballocation(const BufferSuballocation&) = delete;
			BufferSuballocation(BufferSuballocation&& other) :
				buffer{ other.buffer },
				size{ std::exchange(other.size, {}) },
				offset{ std::exchange(other.offset, {}) }
			{

			}

			BufferSuballocation& operator=(const BufferSuballocation&) = delete;
			BufferSuballocation& operator=(BufferSuballocation&&) = delete;

			void Update(std::span<const char> data)
			{
				Update(data.data(), data.size());
			}

			void Update(const char* data, uint64_t data_size)
			{
				assert(data_size <= size);
				buffer.Update(data, data_size, offset);
			}
		private:
			Buffer& buffer;
			uint64_t size{};
			uint64_t offset{};
		};

		class CommandList : public GFXResource
		{
		public:
			virtual ~CommandList() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eCommandList;
			}

			virtual void Reset(CommandPoolResetMode reset_mode) = 0;

			virtual void Begin() = 0;
			virtual void End() = 0;

			// Dynamic states
			virtual void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) = 0;
			virtual void SerScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

			virtual void BindPipeline(Pipeline* pipeline) = 0;
			virtual void BindDescriptorSet() = 0;
			virtual void BindDescriptorSets() = 0;
			virtual void PushConstants(Shader* shader, RootSignature* root_signature, std::span<const char> constant_data) = 0;

			virtual void BindIndexBuffer(Buffer* index_buffer, IndexType index_type, uint64_t offset) = 0;
			virtual void BindVertexBuffers(uint32_t buffer_count, Buffer** vertex_buffers, const uint32_t* strides, const uint64_t* offsets) = 0;

			virtual void Draw(uint32_t vertex_count, uint32_t first_vertex, uint32_t first_instance, uint32_t instance_count) = 0;
			virtual void DrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) = 0;
			virtual void DrawIndirect(const IndirectDrawCommand& draw_command) = 0;

			//virtual void WriteTimestamp(QueryPool* query_pool, uint32_t pool_index, StageFlag stage_flag) = 0;

			virtual void Dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) = 0;
			virtual void DispatchIndirect(Buffer* argument_buffer, uint64_t argument_offset = 0ull) = 0;

			virtual void CopyBuffer(const CopyBufferInfo& copy_info) = 0;
			virtual void CopyTexture(const CopyTextureInfo& copy_info) = 0;

			virtual void ResourceBarrier(const ResourceBarrierInfo& barrier) = 0;

			virtual void ResetQuery(QueryPool* query_pool, uint32_t first_query, uint32_t query_count) = 0;
			virtual void BeginQuery(QueryPool* query_pool, uint32_t query) = 0;
			virtual void EndQuery(QueryPool* query_pool, uint32_t query) = 0;

			virtual void BeginRendering(const RenderingInfo& rendering_info) = 0;
			virtual void EndRendering() = 0;

			virtual void BeginMarker(const char* name, uint32_t color) = 0;
			virtual void InsertMarker(const char* name, uint32_t color) = 0;
			virtual void EndMarker() = 0;
		};

		class CommandPool : public GFXResource
		{
		public:
			virtual ~CommandPool() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eCommandPool;
			}

			virtual CommandList* Allocate(bool secondary = false) = 0;
			virtual void Reset() = 0;
		};

		class Texture : public GFXResource
		{
		public:
			virtual ~Texture() = default;

			// Reset subresource layouts
			virtual void Reset() = 0;

			ResourceType GetResourceType() override
			{
				return ResourceType::eTexture;
			}

			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			virtual uint32_t GetDepth() const = 0;

			virtual uint32_t GetMipLevelCount() const = 0;
			virtual uint32_t GetArrayLayerCount() const = 0;

			virtual Format GetFormat() const = 0;
		};

		class Sampler : public GFXResource
		{
		public:
			virtual ~Sampler() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eSampler;
			}
		};

		class Shader : public GFXResource
		{
		public:
			virtual ~Shader() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eShader;
			}

			ShaderReflection const& GetReflection() const
			{
				return reflection;
			}

		protected:
			ShaderReflection reflection{};
		};

		class RootSignature : public GFXResource
		{
		public:
			virtual ~RootSignature() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eRootSignature;
			}
		};

		class Pipeline : public GFXResource
		{
		public:
			virtual ~Pipeline() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::ePipeline;
			}

			virtual PipelineType GetType() const = 0;
		};

		class PipelineCache : public GFXResource
		{
		public:
			virtual ~PipelineCache() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::ePipelineCache;
			}

			virtual void GetData(size_t& data_size, void* data_ptr) = 0;
		};

		class DescriptorSet : public GFXResource
		{
		public:
			virtual ~DescriptorSet() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eDescriptorSet;
			}

			virtual void Reset() = 0;
			virtual void Update() = 0;
		};

		class DescriptorPool : public GFXResource
		{
		public:
			virtual ~DescriptorPool() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eDescriptorPool;
			}

			virtual DescriptorSet* Allocate() = 0;
		};

		class QueryPool : public GFXResource
		{
		public:
			virtual ~QueryPool() = default;

			ResourceType GetResourceType() override
			{
				return ResourceType::eQueryPool;
			}

			virtual void Reset(uint32_t start_query, uint32_t query_count = 0u) = 0;
			virtual void GetData(uint32_t query_index, void* data) = 0;
			virtual void GetData(uint64_t first_query, uint32_t query_count, void* data) = 0;
		};
	}
}