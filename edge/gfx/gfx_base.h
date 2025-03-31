#pragma once

#include <memory>
#include "gfx_structures.h"

namespace edge::gfx
{
	class IGFXResource {
	public:
		virtual ~IGFXResource() = default;

		virtual auto set_resource_name(std::string_view name) const noexcept -> void = 0;

		[[nodiscard]] virtual auto get_resource_type() const noexcept -> ResourceType = 0;

		template<typename _Ty>
		[[nodiscard]] auto as() -> _Ty {
			//assert();
			return static_cast<_Ty>(this);
		}
	};

	class IFence : public IGFXResource {
	public:
		~IFence() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eFence;
		}

		virtual auto wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) -> void = 0;
		virtual auto reset() -> void = 0;
	};

	class ISemaphore : public IGFXResource
	{
	public:
		~ISemaphore() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eSemaphore;
		}
	};

	class IQueue : public IGFXResource {
	public:
		~IQueue() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eQueue;
		}

		virtual auto submit(const SubmitInfo& submit_info, IFence* fence) const -> void = 0;
		virtual auto present(const PresentInfo& present_info) const -> void = 0;
		virtual auto wait_idle() const -> void = 0;
	};

	class ISwapchain : public IGFXResource {
	public:
		~ISwapchain() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eSwapchain;
		}

		[[nodiscard]] virtual auto get_image_count() const noexcept -> uint32_t = 0;
		[[nodiscard]] virtual auto get_image_format() const noexcept -> Format = 0;

		[[nodiscard]] virtual auto get_width() const noexcept -> uint32_t const& = 0;
		[[nodiscard]] virtual auto get_height() const noexcept -> uint32_t const& = 0;

		virtual auto get_surface_extent(uint32_t& width, uint32_t& height) -> void = 0;

		[[nodiscard]] virtual auto acquire_next_image(ISemaphore* signal_semaphore, IFence* fence, uint32_t* image_index) -> bool = 0;
	};

	class IBuffer : public IGFXResource {
	public:
		~IBuffer() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eBuffer;
		}

		[[nodiscard]] virtual auto map() -> uint8_t* = 0;
		virtual auto unmap() noexcept -> void = 0;

		[[nodiscard]] virtual auto update(const void* data, uint64_t size, uint64_t offset = 0ull) -> uint64_t = 0;

		[[nodiscard]] virtual auto get_size() const noexcept -> uint64_t = 0;
		[[nodiscard]] virtual auto get_address() const->uint64_t = 0;

		[[nodiscard]] virtual auto get_usage_flags() const noexcept -> BufferUsageFlags = 0;
	};

	class BufferSuballocation final {
	public:
		BufferSuballocation(IBuffer& buffer, uint64_t offset, uint64_t size) :
			buffer{ buffer },
			offset{ offset },
			size{ size } {
			mapped = buffer.map();
		}

		~BufferSuballocation() = default;

		BufferSuballocation(const BufferSuballocation&) = delete;
		BufferSuballocation(BufferSuballocation&& other) :
			buffer{ other.buffer },
			mapped{ std::exchange(other.mapped, nullptr) },
			size{ std::exchange(other.size, {}) },
			offset{ std::exchange(other.offset, {}) } {
		}

		BufferSuballocation& operator=(const BufferSuballocation&) = delete;
		BufferSuballocation& operator=(BufferSuballocation&&) = delete;

		template<typename _Ty>
		auto update(std::span<_Ty> data) -> void {
			update(data.data(), data.size() * sizeof(_Ty));
		}

		auto update(const void* data, uint64_t data_size) -> void {
			assert(data_size <= size);
			memcpy(mapped + offset, data, data_size);
			//buffer.Update(data, data_size, offset);
		}

		[[nodiscard]] auto GetBuffer() noexcept -> IBuffer& {
			return buffer;
		}

		[[nodiscard]] auto GetSize() const noexcept -> uint64_t {
			return size;
		}

		[[nodiscard]] auto GetOffset() const noexcept -> uint64_t {
			return offset;
		}
	private:
		IBuffer& buffer;
		uint8_t* mapped{ nullptr };
		uint64_t size{};
		uint64_t offset{};
	};

	class ICommandList : public IGFXResource {
	public:
		~ICommandList() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eCommandList;
		}

		virtual auto reset(CommandPoolResetMode reset_mode) -> void = 0;

		virtual auto begin() -> void = 0;
		virtual auto end() -> void = 0;

		// Dynamic states
		virtual auto set_viewport(float x, float y, float width, float height, float min_depth, float max_depth) const  -> void = 0;
		virtual auto set_scissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const  -> void = 0;

		virtual auto bind_pipeline(IPipeline* pipeline) const -> void = 0;
		virtual auto bind_descriptors(const BindDescriptorsInfo& bind_info) const -> void = 0;
		virtual auto push_constants(IShader* shader, IRootSignature* root_signature, std::span<const char> constant_data) const -> void = 0;

		virtual auto bind_index_buffer(IBuffer* index_buffer, IndexType index_type, uint64_t offset) const -> void = 0;
		virtual auto bind_vertex_buffers(std::span<IBuffer*> vertex_buffers, std::span<const uint32_t> strides, std::span<const uint64_t> offsets) const -> void = 0;

		virtual auto draw(uint32_t vertex_count, uint32_t first_vertex, uint32_t first_instance, uint32_t instance_count) const -> void = 0;
		virtual auto draw_indexed(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const -> void = 0;
		virtual auto draw_indirect(const IndirectDrawCommand& draw_command) const  -> void = 0;

		//virtual void write_timestamp(IQueryPool* query_pool, uint32_t pool_index, StageFlag stage_flag) = 0;

		virtual auto dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) const -> void = 0;
		virtual auto dispatch_indirect(IBuffer* argument_buffer, uint64_t argument_offset = 0ull) const -> void = 0;

		virtual auto copy_buffer(const CopyBufferInfo& copy_info) const -> void = 0;
		virtual auto copy_texture(const CopyTextureInfo& copy_info) const -> void = 0;

		virtual auto resource_barrier(const ResourceBarrierInfo& barrier) const -> void = 0;

		virtual auto reset_query(IQueryPool* query_pool, uint32_t first_query, uint32_t query_count) const -> void = 0;
		virtual auto begin_query(IQueryPool* query_pool, uint32_t query) const -> void = 0;
		virtual auto end_query(IQueryPool* query_pool, uint32_t query) const -> void = 0;

		virtual auto begin_rendering(const RenderingInfo& rendering_info) const -> void = 0;
		virtual auto end_rendering() const -> void = 0;

		virtual auto begin_marker(std::string_view name, uint32_t color) const -> void = 0;
		virtual auto insert_marker(std::string_view name, uint32_t color) const -> void = 0;
		virtual auto end_marker() const -> void = 0;
	};

	class ICommandPool : public IGFXResource {
	public:
		~ICommandPool() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eCommandPool;
		}

		[[nodiscard]] virtual auto allocate(bool secondary = false) -> ICommandList* = 0;
		virtual auto reset() noexcept -> void = 0;
	};

	class ITexture : public IGFXResource {
	public:
		~ITexture() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eTexture;
		}

		[[nodiscard]] virtual auto get_width() const noexcept -> uint32_t = 0;
		[[nodiscard]] virtual auto get_height() const noexcept -> uint32_t = 0;
		[[nodiscard]] virtual auto get_depth() const noexcept -> uint32_t = 0;

		[[nodiscard]] virtual auto get_mip_level_count() const noexcept -> uint32_t = 0;
		[[nodiscard]] virtual auto get_array_layer_count() const noexcept -> uint32_t = 0;

		[[nodiscard]] virtual auto get_format() const noexcept -> Format = 0;
	};

	class ISampler : public IGFXResource {
	public:
		~ISampler() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eSampler;
		}
	};

	class IShader : public IGFXResource {
	public:
		~IShader() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eShader;
		}

		[[nodiscard]] auto get_reflection() const noexcept -> ShaderReflection const& {
			return reflection;
		}

	protected:
		ShaderReflection reflection{};
	};

	class IRootSignature : public IGFXResource {
	public:
		~IRootSignature() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eRootSignature;
		}
	};

	class IPipeline : public IGFXResource {
	public:
		~IPipeline() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::ePipeline;
		}

		[[nodiscard]] virtual auto get_type() const noexcept -> PipelineType = 0;
	};

	class IPipelineCache : public IGFXResource {
	public:
		~IPipelineCache() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::ePipelineCache;
		}

		virtual auto get_data(size_t& data_size, void* data_ptr) const -> void = 0;
	};

	class IResourceDescriptor : public IGFXResource {
	public:
		~IResourceDescriptor() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eResourceDescriptor;
		}

		virtual auto bind_texture(TextureDescriptor& texture_descriptor) -> void = 0;
		virtual auto bind_buffer(BufferDescriptor& buffer_descriptor) -> void = 0;

		virtual auto flush() -> void = 0;
	};

	class IDescriptorAllocator : public IGFXResource {
	public:
		~IDescriptorAllocator() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eDescriptorAllocator;
		}

		virtual void reset() = 0;
		[[nodiscard]] virtual auto allocate_descriptors(IRootSignature* root_signature) -> IResourceDescriptor* = 0;
	};

	class ISharedDescriptor : public IGFXResource {
	public:
		~ISharedDescriptor() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eSharedDescriptor;
		}

		virtual void reset() = 0;
		[[nodiscard]] virtual auto get_latest_descriptors() const noexcept -> IResourceDescriptor* = 0;
		[[nodiscard]] virtual auto request_descriptors() -> IResourceDescriptor* = 0;
	};

	class IQueryPool : public IGFXResource {
	public:
		~IQueryPool() override = default;

		[[nodiscard]] auto get_resource_type() const noexcept -> ResourceType override final {
			return ResourceType::eQueryPool;
		}

		virtual auto reset(uint32_t start_query, uint32_t query_count = 0u) -> void = 0;
		virtual auto get_data(uint32_t query_index, void* data) const -> void = 0;
		virtual auto get_data(uint64_t first_query, uint32_t query_count, void* data) const -> void = 0;
	};
}