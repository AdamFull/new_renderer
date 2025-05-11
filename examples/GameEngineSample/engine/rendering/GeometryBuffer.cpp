#include "GeometryBuffer.h"

#include <edge/gfx/gfx_context.h>
#include <edge/util/utl_base.h>
#include <edge/util/utl_hash.hpp>
#include <edge/util/utl_profiler.h>

namespace engine::render {
	inline auto get_vertex_format_size_bytes(edge::gfx::Format format) -> uint32_t {
		switch (format) {
		case edge::gfx::Format::eR8G8Uint:
		case edge::gfx::Format::eR8G8Unorm:
		case edge::gfx::Format::eR16Uint:
		case edge::gfx::Format::eR16Sint:
		case edge::gfx::Format::eR16Sfloat: return 2u; break;
		case edge::gfx::Format::eR32Uint:
		case edge::gfx::Format::eR32Sint:
		case edge::gfx::Format::eR32Sfloat:
		case edge::gfx::Format::eR16G16Uint:
		case edge::gfx::Format::eR16G16Sint:
		case edge::gfx::Format::eR16G16Sfloat:
		case edge::gfx::Format::eR8G8B8A8Unorm: return 4u; break;
		case edge::gfx::Format::eR16G16B16Uint: 
		case edge::gfx::Format::eR16G16B16Sint: 
		case edge::gfx::Format::eR16G16B16Sfloat: return 6u; break;
		case edge::gfx::Format::eR32G32Sfloat:
		case edge::gfx::Format::eR16G16B16A16Unorm: return 8u; break;
		case edge::gfx::Format::eR32G32B32Sfloat: return 12u; break;
		case edge::gfx::Format::eR32G32B32A32Sfloat: return 16u; break;
		default:
			break;
		}

		return 1u;
	}

	inline auto GetVertexAttributeLocation(VertexAttributeFlag flag) -> uint32_t {
		switch (flag) {
		case engine::render::VertexAttributeFlag::ePosition: return 0u; break;
		case engine::render::VertexAttributeFlag::eColor: return 1u; break;
		case engine::render::VertexAttributeFlag::eTexCoord: return 2u; break;
		case engine::render::VertexAttributeFlag::eNormal: return 3u; break;
		case engine::render::VertexAttributeFlag::eTangent: return 4u; break;
		case engine::render::VertexAttributeFlag::eJoint: return 5u; break;
		case engine::render::VertexAttributeFlag::eWeight: return 6u; break;
		default:
			break;
		}

		return ~0u;
	}

	auto hash(const AttributeDescription& desc, uint32_t seed) -> uint32_t {
		seed = edge::murmur_hash(edge::utl::cast(desc.attribute_flag), seed);
		return edge::murmur_hash(edge::utl::cast(desc.format), seed);
	}

	auto hash(const IndexDescription& desc, uint32_t seed) -> uint32_t {
		return edge::murmur_hash(edge::utl::cast(desc.type), seed);
	}

	auto hash(const GeometryBufferCreateInfo& create_info, uint32_t seed) -> uint32_t {
		for (auto& desc : create_info.attribute_descriptions) {
			seed = hash(desc, seed);
		}

		return hash(create_info.index_description, seed);
	}



	GeometryBuffer::GeometryBuffer() {
	}

	GeometryBuffer::~GeometryBuffer() {
		edge::gfx::destroy(index_data.buffer);
	}

	void GeometryBuffer::flush()
	{
		EDGE_PROFILE_ZONE_SCOPED;

		if (!index_data.data.empty()) {
			try_flush_buffer(index_data, edge::gfx::BufferUsageFlag::eIndexBuffer);
			index_data.data.clear();
		}
	}

	void GeometryBuffer::bind_index_buffer(edge::gfx::ICommandList& command_list) {
		if (index_data.buffer) {
			if (index_data.state != edge::gfx::ResourceStateFlag::eIndexRead) {
				edge::gfx::BufferBarrierDesc buffer_barrier{};
				buffer_barrier.buffer = index_data.buffer;
				buffer_barrier.src_state = index_data.state;
				index_data.state = buffer_barrier.dst_state = edge::gfx::ResourceStateFlag::eIndexRead;
				buffer_barrier.range.size = index_data.buffer->get_size();

				edge::gfx::ResourceBarrierInfo resource_barrier{};
				resource_barrier.buffer_barriers = { &buffer_barrier, 1 };

				command_list.resource_barrier(resource_barrier);
			}

			command_list.bind_index_buffer(index_data.buffer, index_data.index_type, 0ull);
		}
	}

	void GeometryBuffer::bind(edge::gfx::ICommandList& command_list) {
		bind_index_buffer(command_list);
		bind_vertex_buffers(command_list);
	}

	uint64_t GeometryBuffer::get_index_buffer_address() {
		return index_data.buffer->get_address();
	}

	void GeometryBuffer::add_index_stream_data(const void* data, uint32_t count, uint32_t stride) {
		index_data.data.insert(index_data.data.end(), static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + count * stride);
		index_data.count += count;
	}

	edge::gfx::VertexLayout const& GeometryBuffer::get_vertex_layout() const {
		return vertex_layout;
	}

	void GeometryBuffer::try_flush_buffer(GeometryData& geometry_data, edge::gfx::BufferUsageFlags usage) {
		EDGE_PROFILE_ZONE_SCOPED;

		if (!geometry_data.buffer || geometry_data.buffer->get_size() < geometry_data.data.size()) {
			if (geometry_data.buffer) {
				edge::gfx::destroy(geometry_data.buffer);
			}

			edge::gfx::BufferCreateInfo create_info{};
			create_info.size = geometry_data.data.size();
			create_info.usage = usage;

			geometry_data.buffer = edge::gfx::create_buffer(create_info);
		}

		edge::gfx::update_buffer(geometry_data.buffer, geometry_data.state, geometry_data.data);
	}



	InterleavedGeometryBuffer::InterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) :
		GeometryBuffer{} {
		uint32_t offset{ 0u };

		for (auto& desc : create_info.attribute_descriptions) {
			auto& attr_desc = vertex_layout.attributes[vertex_layout.attribute_count++];
			attr_desc.binding = 0;
			attr_desc.format = desc.format;
			attr_desc.offset = offset;
			attr_desc.location = GetVertexAttributeLocation(desc.attribute_flag);

			offset += get_vertex_format_size_bytes(desc.format);
		}

		auto& binding = vertex_layout.bindings[vertex_layout.binding_count++];
		binding.binding = 0;
		binding.stride = offset;

		index_data.index_type = create_info.index_description.type;
	}

	InterleavedGeometryBuffer::~InterleavedGeometryBuffer() {
		edge::gfx::destroy(vertex_data.buffer);
	}

	void InterleavedGeometryBuffer::flush() {
		EDGE_PROFILE_ZONE_SCOPED;

		GeometryBuffer::flush();

		if (!vertex_data.data.empty()) {
			try_flush_buffer(vertex_data, edge::gfx::BufferUsageFlag::eVertexBuffer);
			vertex_data.data.clear();
		}
	}

	void InterleavedGeometryBuffer::bind_vertex_buffers(edge::gfx::ICommandList& command_list) {
		const uint32_t stride{ 0u };
		uint64_t offset{ 0ull };

		if (vertex_data.state != edge::gfx::ResourceStateFlag::eVertexRead) {
			edge::gfx::BufferBarrierDesc buffer_barrier{};
			buffer_barrier.buffer = vertex_data.buffer;
			buffer_barrier.src_state = vertex_data.state;
			vertex_data.state = buffer_barrier.dst_state = edge::gfx::ResourceStateFlag::eVertexRead;
			buffer_barrier.range.size = vertex_data.buffer->get_size();

			edge::gfx::ResourceBarrierInfo resource_barrier{};
			resource_barrier.buffer_barriers = { &buffer_barrier, 1 };

			command_list.resource_barrier(resource_barrier);
		}

		assert(vertex_data.buffer && "VERTEX BUFFER IS NOT ALLOCATED");
		command_list.bind_vertex_buffers({ &vertex_data.buffer, 1u }, { &stride, 1u }, { &offset, 1u });
	}

	uint64_t InterleavedGeometryBuffer::get_buffer_address(VertexAttributeFlag flag) {
		return vertex_data.buffer->get_address();
	}

	VertexRange InterleavedGeometryBuffer::add_mesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices) {
		VertexRange vertex_range{};
		vertex_range.first_index = static_cast<uint32_t>(index_data.count);
		vertex_range.index_count = static_cast<uint32_t>(indices.count);

		vertex_range.first_vertex = static_cast<uint32_t>(vertex_data.count);
		vertex_range.vertex_count = static_cast<uint32_t>(attributes[0].count);

		// TODO: implement vertex merging

		if (!indices.byte_range.empty()) {
			index_data.data.insert(index_data.data.end(), indices.byte_range.begin(), indices.byte_range.end());
			index_data.count += indices.count;
		}

		assert(false && "NOT SUPPORTED FOR INTERLEAVED FOR NOW");

		return vertex_range;
	}

	void InterleavedGeometryBuffer::add_vertex_stream_data(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag) {
		// TODO: assert here when stride is not correct
		vertex_data.data.insert(vertex_data.data.end(), static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + count * stride);
		vertex_data.count += count;
	}


	DeinterleavedGeometryBuffer::DeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) {
		for (auto& desc : create_info.attribute_descriptions) {
			auto& attr_desc = vertex_layout.attributes[vertex_layout.attribute_count++];
			attr_desc.binding = vertex_layout.binding_count;
			attr_desc.format = desc.format;
			attr_desc.location = GetVertexAttributeLocation(desc.attribute_flag);

			auto& bind_desc = vertex_layout.bindings[vertex_layout.binding_count++];
			bind_desc.binding = attr_desc.binding;
			bind_desc.stride = get_vertex_format_size_bytes(desc.format);
		}

		//index_data.type = create_info.index_description.type;
	}

	DeinterleavedGeometryBuffer::~DeinterleavedGeometryBuffer() {
		for (auto& vert_data : vertex_datas) {
			edge::gfx::destroy(vert_data.buffer);
		}
	}

	void DeinterleavedGeometryBuffer::flush() {
		EDGE_PROFILE_ZONE_SCOPED;

		GeometryBuffer::flush();

		for (auto& vertex_data : vertex_datas) {
			if (!vertex_data.data.empty()) {
				try_flush_buffer(vertex_data, edge::gfx::BufferUsageFlag::eVertexBuffer);
				vertex_data.data.clear();
			}
		}
	}

	void DeinterleavedGeometryBuffer::bind_vertex_buffers(edge::gfx::ICommandList& command_list) {
		uint32_t buffer_count{ 0u };
		std::array<edge::gfx::IBuffer*, k_max_attribure_description> vertex_buffers{};
		std::array<uint32_t, k_max_attribure_description> strides{};
		std::array<uint64_t, k_max_attribure_description> offsets{};
		std::array<edge::gfx::BufferBarrierDesc, k_max_attribure_description> barriers{};
		size_t barrier_count{ 0 };

		edge::gfx::ResourceBarrierInfo resource_barrier{};

		for (auto& vert_data : vertex_datas) {
			if (vert_data.buffer) {
				if (vert_data.state != edge::gfx::ResourceStateFlag::eVertexRead) {
					auto& barrier = barriers[barrier_count++];
					barrier.buffer = vert_data.buffer;
					barrier.src_state = vert_data.state;
					vert_data.state = barrier.dst_state = edge::gfx::ResourceStateFlag::eVertexRead;
					barrier.range.size = vert_data.buffer->get_size();
				}

				vertex_buffers[buffer_count++] = vert_data.buffer;
			}
		}

		resource_barrier.buffer_barriers = { barriers.data(), barrier_count };

		command_list.resource_barrier(resource_barrier);
		command_list.bind_vertex_buffers({ vertex_buffers.data(), buffer_count }, { strides.data(), buffer_count }, { offsets.data(), buffer_count });
	}

	auto DeinterleavedGeometryBuffer::get_buffer_address(VertexAttributeFlag flag) -> uint64_t {
		uint32_t index = GetVertexAttributeLocation(flag);
		assert(vertex_datas[index].buffer && "BUFFER IS NOT ALLOCATED");
		return vertex_datas[index].buffer->get_address();
	}

	template<typename _From, typename _To, typename _Pred>
	void transform(GeometryData& vdat, const AttributeStream& attr, _Pred&& pred) {
		auto* begin = reinterpret_cast<const _From*>(attr.byte_range.data());
		auto* end = begin + attr.count;

		std::vector<_To> encoded{};
		for (auto it = begin; it != end; ++it) {
			encoded.emplace_back(pred(*it));
		}

		auto* enc_begin = reinterpret_cast<const uint8_t*>(encoded.data());
		auto* enc_end = enc_begin + attr.count * sizeof(_To);

		vdat.data.insert(vdat.data.end(), enc_begin, enc_end);
	}

	auto DeinterleavedGeometryBuffer::add_mesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices) -> VertexRange {
		VertexRange vertex_range{};
		vertex_range.first_index = static_cast<uint32_t>(index_data.count);
		vertex_range.index_count = static_cast<uint32_t>(indices.count);

		vertex_range.first_vertex = static_cast<uint32_t>(vertex_datas[0].count);
		vertex_range.vertex_count = static_cast<uint32_t>(attributes[0].count);

		for (auto& attribute : attributes) {
			if (attribute.byte_range.empty()) {
				continue;
			}

			uint32_t index = GetVertexAttributeLocation(attribute.attribute_flag);
			auto& vert_data = vertex_datas[index];
			vert_data.data.insert(vert_data.data.end(), attribute.byte_range.begin(), attribute.byte_range.end());
			vert_data.count += attribute.count;
		}

		if (!indices.byte_range.empty()) {
			index_data.data.insert(index_data.data.end(), indices.byte_range.begin(), indices.byte_range.end());
			index_data.count += indices.count;
		}

		return vertex_range;
	}

	void DeinterleavedGeometryBuffer::add_vertex_stream_data(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag) {
		uint32_t index = GetVertexAttributeLocation(flag);
		auto& vert_data = vertex_datas[index];
		vert_data.data.insert(vert_data.data.end(), static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + count * stride);
		vert_data.count += count;
	}
}