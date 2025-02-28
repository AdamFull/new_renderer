#pragma once

#include <memory>
#include "gfx_structures.h"

namespace edge
{
	namespace gfx
	{
		class IGFXResource
		{
		public:
			virtual ~IGFXResource() = default;

			virtual void SetResourceName(std::string_view name) const noexcept = 0;

			[[nodiscard]] virtual auto GetResourceType() const noexcept -> ResourceType = 0;
		};

		class IRenderResource : public IGFXResource
		{
		public:
			~IRenderResource() override = default;

			[[nodiscard]] auto GetResourceState() const noexcept -> ResourceStateFlags 
			{
				return state;
			}
		protected:
			ResourceStateFlags state{ ResourceStateFlag::eUndefined };
		};

		class IFence : public IGFXResource
		{
		public:
			~IFence() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final 
			{
				return ResourceType::eFence;
			}

			virtual void Wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) = 0;
			virtual void Reset() = 0;
		};

		class ISemaphore : public IGFXResource
		{
		public:
			~ISemaphore() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eSemaphore;
			}
		};

		class IQueue : public IGFXResource
		{
		public:
			~IQueue() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eQueue;
			}

			virtual void Submit(const SubmitInfo& submit_info, IFence* fence) const = 0;
			virtual void Present(const PresentInfo& present_info) const = 0;
			virtual void WaitIdle() const = 0;
		};

		class ISwapchain : public IGFXResource
		{
		public:
			~ISwapchain() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eSwapchain;
			}

			[[nodiscard]] virtual auto GetImageCount() const noexcept -> uint32_t = 0;
			[[nodiscard]] virtual auto GetImageFormat() const noexcept -> Format = 0;

			[[nodiscard]] virtual auto GetWidth() const noexcept -> uint32_t const& = 0;
			[[nodiscard]] virtual auto GetHeight() const noexcept -> uint32_t const& = 0;
				
			virtual void GetSurfaceExtent(uint32_t& width, uint32_t& height) = 0;

			[[nodiscard]] virtual auto AcquireNextImage(ISemaphore* signal_semaphore, IFence* fence, uint32_t* image_index) -> bool = 0;
		};

		class IBuffer : public IRenderResource
		{
		public:
			~IBuffer() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eBuffer;
			}

			[[nodiscard]] virtual auto Map() -> uint8_t* = 0;
			virtual void Unmap() noexcept = 0;

			[[nodiscard]] virtual auto Update(const void* data, uint64_t size, uint64_t offset = 0ull) -> uint64_t = 0;

			[[nodiscard]] virtual auto GetSize() const noexcept -> uint64_t = 0;
			[[nodiscard]] virtual auto GetAddress() const -> uint64_t = 0;

			[[nodiscard]] virtual auto GetUsageFlags() const noexcept -> BufferUsageFlags = 0;
		};

		class BufferSuballocation final
		{
		public:
			BufferSuballocation(IBuffer& buffer, uint64_t offset, uint64_t size) :
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

			template<typename _Ty>
			void Update(std::span<_Ty> data)
			{
				Update(data.data(), data.size() * sizeof(_Ty));
			}

			void Update(const void* data, uint64_t data_size)
			{
				assert(data_size <= size);
				buffer.Update(data, data_size, offset);
			}

			[[nodiscard]] auto GetBuffer() noexcept -> IBuffer&
			{
				return buffer;
			}

			[[nodiscard]] auto GetSize() const noexcept -> uint64_t
			{
				return size;
			}

			[[nodiscard]] auto GetOffset() const noexcept -> uint64_t
			{
				return offset;
			}
		private:
			IBuffer& buffer;
			uint64_t size{};
			uint64_t offset{};
		};

		class ICommandList : public IGFXResource
		{
		public:
			~ICommandList() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eCommandList;
			}

			virtual void Reset(CommandPoolResetMode reset_mode) = 0;

			virtual void Begin() = 0;
			virtual void End() = 0;

			// Dynamic states
			virtual void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) const = 0;
			virtual void SerScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const = 0;

			virtual void BindPipeline(IPipeline* pipeline) const = 0;
			virtual void BindDescriptors(const BindDescriptorsInfo& bind_info) const = 0;
			virtual void PushConstants(IShader* shader, IRootSignature* root_signature, std::span<const char> constant_data) const = 0;

			virtual void BindIndexBuffer(IBuffer* index_buffer, IndexType index_type, uint64_t offset) const = 0;
			virtual void BindVertexBuffers(std::span<IBuffer*> vertex_buffers, std::span<const uint32_t> strides, std::span<const uint64_t> offsets) const = 0;

			virtual void Draw(uint32_t vertex_count, uint32_t first_vertex, uint32_t first_instance, uint32_t instance_count) const = 0;
			virtual void DrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const = 0;
			virtual void DrawIndirect(const IndirectDrawCommand& draw_command) const = 0;

			//virtual void WriteTimestamp(IQueryPool* query_pool, uint32_t pool_index, StageFlag stage_flag) = 0;

			virtual void Dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) const = 0;
			virtual void DispatchIndirect(IBuffer* argument_buffer, uint64_t argument_offset = 0ull) const = 0;

			virtual void CopyBuffer(const CopyBufferInfo& copy_info) const = 0;
			virtual void CopyTexture(const CopyTextureInfo& copy_info) const = 0;

			virtual void ResourceBarrier(const ResourceBarrierInfo& barrier) const = 0;

			virtual void ResetQuery(IQueryPool* query_pool, uint32_t first_query, uint32_t query_count) const = 0;
			virtual void BeginQuery(IQueryPool* query_pool, uint32_t query) const = 0;
			virtual void EndQuery(IQueryPool* query_pool, uint32_t query) const = 0;

			virtual void BeginRendering(const RenderingInfo& rendering_info) const = 0;
			virtual void EndRendering() const = 0;

			virtual void BeginMarker(std::string_view name, uint32_t color) const = 0;
			virtual void InsertMarker(std::string_view name, uint32_t color) const = 0;
			virtual void EndMarker() const = 0;
		};

		class ICommandPool : public IGFXResource
		{
		public:
			~ICommandPool() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eCommandPool;
			}

			[[nodiscard]] virtual auto Allocate(bool secondary = false) -> ICommandList* = 0;
			virtual void Reset() noexcept = 0;
		};

		class ITexture : public IRenderResource
		{
		public:
			~ITexture() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eTexture;
			}

			[[nodiscard]] virtual auto GetWidth() const noexcept -> uint32_t = 0;
			[[nodiscard]] virtual auto GetHeight() const noexcept -> uint32_t = 0;
			[[nodiscard]] virtual auto GetDepth() const noexcept -> uint32_t = 0;

			[[nodiscard]] virtual auto GetMipLevelCount() const noexcept -> uint32_t = 0;
			[[nodiscard]] virtual auto GetArrayLayerCount() const noexcept -> uint32_t = 0;

			[[nodiscard]] virtual auto GetFormat() const noexcept -> Format = 0;
		};

		class ISampler : public IGFXResource
		{
		public:
			~ISampler() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eSampler;
			}
		};

		class IShader : public IGFXResource
		{
		public:
			~IShader() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eShader;
			}

			[[nodiscard]] auto GetReflection() const noexcept -> ShaderReflection const&
			{
				return reflection;
			}

		protected:
			ShaderReflection reflection{};
		};

		class IShaderEffect
		{
		public:
			virtual ~IShaderEffect() = default;

			[[nodiscard]] virtual auto GetShader() noexcept -> IShader* = 0;
		};

		class IRootSignature : public IGFXResource
		{
		public:
			~IRootSignature() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eRootSignature;
			}
		};

		class IPipeline : public IGFXResource
		{
		public:
			~IPipeline() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::ePipeline;
			}

			[[nodiscard]] virtual auto GetType() const noexcept -> PipelineType = 0;
		};

		class IPipelineCache : public IGFXResource
		{
		public:
			~IPipelineCache() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::ePipelineCache;
			}

			virtual void GetData(size_t& data_size, void* data_ptr) = 0;
		};

		class IResourceDescriptor : public IGFXResource
		{
		public:
			~IResourceDescriptor() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eResourceDescriptor;
			}

			virtual void BindTexture(TextureDescriptor& texture_descriptor) = 0;
			virtual void BindBuffer(BufferDescriptor& buffer_descriptor) = 0;

			virtual void Flush() = 0;
		};

		class IDescriptorAllocator : public IGFXResource
		{
		public:
			~IDescriptorAllocator() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eDescriptorAllocator;
			}

			virtual void Reset() = 0;
			[[nodiscard]] virtual auto AllocateDescriptors(IRootSignature* root_signature) -> IResourceDescriptor* = 0;
		};

		class ISharedDescriptor : public IGFXResource
		{
		public:
			~ISharedDescriptor() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eSharedDescriptor;
			}

			virtual void Reset() = 0;
			[[nodiscard]] virtual auto GetLatestDescriptors() const noexcept -> IResourceDescriptor* = 0;
			[[nodiscard]] virtual auto RequestDescriptors() -> IResourceDescriptor* = 0;
		};

		class IQueryPool : public IGFXResource
		{
		public:
			~IQueryPool() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eQueryPool;
			}

			virtual void Reset(uint32_t start_query, uint32_t query_count = 0u) = 0;
			virtual void GetData(uint32_t query_index, void* data) = 0;
			virtual void GetData(uint64_t first_query, uint32_t query_count, void* data) = 0;
		};
	}
}