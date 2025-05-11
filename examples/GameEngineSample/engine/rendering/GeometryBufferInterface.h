#pragma once

#include <edge/gfx/gfx_resource_loader.h>

#include <unordered_map>

namespace engine::render {
	static constexpr uint32_t k_max_attribure_description{ 7u };

	enum class VertexAttributeFlag : uint32_t {
		ePosition = 1 << 0,
		eColor = 1 << 1,
		eTexCoord = 1 << 2,
		eNormal = 1 << 3,
		eTangent = 1 << 4,
		eJoint = 1 << 5,
		eWeight = 1 << 6
	};
	EDGE_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(VertexAttributeFlags, VertexAttributeFlag);

	struct AttributeDescription {
		VertexAttributeFlag attribute_flag{};
		edge::gfx::Format format{};
	};

	struct IndexDescription {
		edge::gfx::IndexType type{ edge::gfx::IndexType::eUint16 };
	};

	struct GeometryBufferCreateInfo {
		std::vector<AttributeDescription> attribute_descriptions{};
		IndexDescription index_description{};
	};

	struct GeometryData {
		size_t count{ 0 };
		edge::gfx::IBuffer* buffer{ nullptr };
		edge::gfx::ResourceStateFlags state{};
		std::vector<uint8_t> data{};

		edge::gfx::IndexType index_type{};
	};

	struct AttributeStream {
		VertexAttributeFlag attribute_flag{};
		std::span<const uint8_t> byte_range{};
		size_t count{ 0 };
	};

	struct IndexStream {
		std::span<const uint8_t> byte_range{};
		size_t count{ 0 };
	};

	struct VertexRange {
		uint32_t first_index{};
		uint32_t index_count{};
		uint32_t first_vertex{};
		uint32_t vertex_count{};
	};

	uint32_t hash(const AttributeDescription& desc, uint32_t hash = 0u);
	uint32_t hash(const IndexDescription& desc, uint32_t hash = 0u);
	uint32_t hash(const GeometryBufferCreateInfo& create_info, uint32_t hash = 0u);

	class GeometryBuffer {
	public:
		GeometryBuffer();
		virtual ~GeometryBuffer();

		virtual void flush();

		void bind_index_buffer(edge::gfx::ICommandList& command_list);

		virtual void bind_vertex_buffers(edge::gfx::ICommandList& command_list) = 0;

		void bind(edge::gfx::ICommandList& command_list);

		virtual auto get_buffer_address(VertexAttributeFlag flag = {}) -> uint64_t = 0;

		auto get_index_buffer_address() -> uint64_t;

		virtual auto add_mesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices) -> VertexRange = 0;
		virtual void add_vertex_stream_data(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag = {}) = 0;

		void add_index_stream_data(const void* data, uint32_t count, uint32_t stride);

		auto get_vertex_layout() const-> edge::gfx::VertexLayout const&;
	protected:
		void try_flush_buffer(GeometryData& geometry_data, edge::gfx::BufferUsageFlags usage);
	protected:
		edge::gfx::VertexLayout vertex_layout{};
		GeometryData index_data{};
	};
}