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

			virtual auto SetResourceName(std::string_view name) const noexcept -> void = 0;

			[[nodiscard]] virtual auto GetResourceType() const noexcept -> ResourceType = 0;

			template<typename _Ty>
			[[nodiscard]] auto As() -> _Ty
			{
				//assert();
				return static_cast<_Ty>(this);
			}
		};

		class IFence : public IGFXResource
		{
		public:
			~IFence() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final 
			{
				return ResourceType::eFence;
			}

			virtual auto Wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) -> void = 0;
			virtual auto Reset() -> void = 0;
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

			virtual auto Submit(const SubmitInfo& submit_info, IFence* fence) const -> void = 0;
			virtual auto Present(const PresentInfo& present_info) const -> void = 0;
			virtual auto WaitIdle() const -> void = 0;
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
				
			virtual auto GetSurfaceExtent(uint32_t& width, uint32_t& height) -> void = 0;

			[[nodiscard]] virtual auto AcquireNextImage(ISemaphore* signal_semaphore, IFence* fence, uint32_t* image_index) -> bool = 0;
		};

		class IBuffer : public IGFXResource
		{
		public:
			~IBuffer() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eBuffer;
			}

			[[nodiscard]] virtual auto Map() -> uint8_t* = 0;
			virtual auto Unmap() noexcept -> void = 0;

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
				mapped = buffer.Map();
			}

			~BufferSuballocation() = default;

			BufferSuballocation(const BufferSuballocation&) = delete;
			BufferSuballocation(BufferSuballocation&& other) :
				buffer{ other.buffer },
				mapped{ std::exchange(other.mapped, nullptr) },
				size{ std::exchange(other.size, {}) },
				offset{ std::exchange(other.offset, {}) }
			{
			}

			BufferSuballocation& operator=(const BufferSuballocation&) = delete;
			BufferSuballocation& operator=(BufferSuballocation&&) = delete;

			template<typename _Ty>
			auto Update(std::span<_Ty> data) -> void
			{
				Update(data.data(), data.size() * sizeof(_Ty));
			}

			auto Update(const void* data, uint64_t data_size) -> void
			{
				assert(data_size <= size);
				memcpy(mapped + offset, data, data_size);
				//buffer.Update(data, data_size, offset);
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
			uint8_t* mapped{ nullptr };
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

			virtual auto Reset(CommandPoolResetMode reset_mode) -> void = 0;

			virtual auto Begin() -> void = 0;
			virtual auto End() -> void = 0;

			// Dynamic states
			virtual auto SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) const  -> void = 0;
			virtual auto SerScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const  -> void = 0;

			virtual auto BindPipeline(IPipeline* pipeline) const -> void = 0;
			virtual auto BindDescriptors(const BindDescriptorsInfo& bind_info) const -> void = 0;
			virtual auto PushConstants(IShader* shader, IRootSignature* root_signature, std::span<const char> constant_data) const -> void = 0;

			virtual auto BindIndexBuffer(IBuffer* index_buffer, IndexType index_type, uint64_t offset) const -> void = 0;
			virtual auto BindVertexBuffers(std::span<IBuffer*> vertex_buffers, std::span<const uint32_t> strides, std::span<const uint64_t> offsets) const -> void = 0;

			virtual auto Draw(uint32_t vertex_count, uint32_t first_vertex, uint32_t first_instance, uint32_t instance_count) const -> void = 0;
			virtual auto DrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const -> void = 0;
			virtual auto DrawIndirect(const IndirectDrawCommand& draw_command) const  -> void = 0;

			//virtual void WriteTimestamp(IQueryPool* query_pool, uint32_t pool_index, StageFlag stage_flag) = 0;

			virtual auto Dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) const -> void = 0;
			virtual auto DispatchIndirect(IBuffer* argument_buffer, uint64_t argument_offset = 0ull) const -> void = 0;

			virtual auto CopyBuffer(const CopyBufferInfo& copy_info) const -> void = 0;
			virtual auto CopyTexture(const CopyTextureInfo& copy_info) const -> void = 0;

			virtual auto ResourceBarrier(const ResourceBarrierInfo& barrier) const -> void = 0;

			virtual auto ResetQuery(IQueryPool* query_pool, uint32_t first_query, uint32_t query_count) const -> void = 0;
			virtual auto BeginQuery(IQueryPool* query_pool, uint32_t query) const -> void = 0;
			virtual auto EndQuery(IQueryPool* query_pool, uint32_t query) const -> void = 0;

			virtual auto BeginRendering(const RenderingInfo& rendering_info) const -> void = 0;
			virtual auto EndRendering() const -> void = 0;

			virtual auto BeginMarker(std::string_view name, uint32_t color) const -> void = 0;
			virtual auto InsertMarker(std::string_view name, uint32_t color) const -> void = 0;
			virtual auto EndMarker() const -> void = 0;
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
			virtual auto Reset() noexcept -> void = 0;
		};

		class ITexture : public IGFXResource
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

			virtual auto GetData(size_t& data_size, void* data_ptr) const -> void = 0;
		};

		class IResourceDescriptor : public IGFXResource
		{
		public:
			~IResourceDescriptor() override = default;

			[[nodiscard]] auto GetResourceType() const noexcept -> ResourceType override final
			{
				return ResourceType::eResourceDescriptor;
			}

			virtual auto BindTexture(TextureDescriptor& texture_descriptor) -> void = 0;
			virtual auto BindBuffer(BufferDescriptor& buffer_descriptor) -> void = 0;

			virtual auto Flush() -> void = 0;
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

			virtual auto Reset(uint32_t start_query, uint32_t query_count = 0u) -> void = 0;
			virtual auto GetData(uint32_t query_index, void* data) const -> void = 0;
			virtual auto GetData(uint64_t first_query, uint32_t query_count, void* data) const -> void = 0;
		};
	}
}