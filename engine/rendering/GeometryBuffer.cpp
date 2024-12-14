#include "GeometryBuffer.h"

#include <edge/gfx/gfx_context.h>

#include <edge/util/util.h>

#include <hash.hpp>

#include <edge/util/Profiler.h>

#include <embed/shaders/includes/geometry.h>

namespace engine
{
	namespace render
	{
		inline uint32_t GetVertexFormatSizeBytes(edge::gfx::Format format)
		{
			switch (format)
			{
			case edge::gfx::Format::eR8G8B8A8Unorm: return 4u; break;
			case edge::gfx::Format::eR32G32Sfloat:
			case edge::gfx::Format::eR16G16B16A16Unorm: return 8u; break;
			case edge::gfx::Format::eR32G32B32Sfloat: return 12u; break;
			case edge::gfx::Format::eR32G32B32A32Sfloat: return 16u; break;
			default:
				break;
			}

			return 1u;
		}

		inline uint32_t GetVertexAttributeLocation(VertexAttributeFlag flag)
		{
			switch (flag)
			{
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

		uint32_t Hash(const AttributeDescription& desc, uint32_t hash)
		{
			hash = utl::murmur_hash(utl::enum_cast(desc.attribute_flag), hash);
			hash = utl::murmur_hash(utl::enum_cast(desc.format), hash);

			return hash;
		}

		uint32_t Hash(const IndexDescription& desc, uint32_t hash)
		{
			hash = utl::murmur_hash(utl::enum_cast(desc.type), hash);
			return hash;
		}

		uint32_t Hash(const GeometryBufferCreateInfo& create_info, uint32_t hash)
		{
			for (auto& desc : create_info.attribute_descriptions)
			{
				hash = Hash(desc, hash);
			}

			hash = Hash(create_info.index_description, hash);

			return hash;
		}



		GeometryBuffer::GeometryBuffer(edge::gfx::ResourceLoader& resource_loader) :
			resource_loader{ resource_loader }
		{
		}

		GeometryBuffer::~GeometryBuffer()
		{
			auto& context = resource_loader.GetContext();
			context.Destroy(index_data.buffer);
		}

		void GeometryBuffer::Flush()
		{
			EDGE_PROFILE_ZONE_SCOPED;

			if (!index_data.data.empty())
			{
				TryFlushBuffer(&index_data.buffer, index_data.data, edge::gfx::BufferUsageFlag::eIndexBuffer);
				index_data.data.clear();
			}
		}

		void GeometryBuffer::BindIndexBuffer(edge::gfx::CommandList& command_list)
		{
			if (index_data.buffer)
			{
				command_list.BindIndexBuffer(index_data.buffer, index_data.type, 0ull);
			}
		}

		void GeometryBuffer::Bind(edge::gfx::CommandList& command_list)
		{
			BindIndexBuffer(command_list);
			BindVertexBuffers(command_list);
		}

		uint64_t GeometryBuffer::GetIndexBufferAddress()
		{
			return index_data.buffer->GetAddress();
		}

		void GeometryBuffer::AddIndexStreamData(const void* data, uint32_t count, uint32_t stride)
		{
			index_data.data.insert(index_data.data.end(), static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + count * stride);
			index_data.count += count;
		}

		edge::gfx::VertexLayout const& GeometryBuffer::GetVertexLayout() const
		{
			return vertex_layout;
		}

		void GeometryBuffer::TryFlushBuffer(edge::gfx::Buffer** buffer_ptr, const std::vector<uint8_t>& data, edge::gfx::BufferUsageFlags usage)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			edge::gfx::Context& context = resource_loader.GetContext();

			if (!*buffer_ptr)
			{
				edge::gfx::BufferCreateInfo create_info{};
				create_info.size = data.size();
				create_info.count = 1ull;
				create_info.usage = usage;

				*buffer_ptr = context.CreateBuffer(create_info);
			}

			if ((*buffer_ptr)->GetSize() < data.size())
			{
				context.Destroy(*buffer_ptr);

				edge::gfx::BufferCreateInfo create_info{};
				create_info.size = data.size();
				create_info.count = 1ull;
				create_info.usage = usage;

				*buffer_ptr = context.CreateBuffer(create_info);
			}

			edge::gfx::BufferUpdateInfo buffer_update_info{};
			buffer_update_info.dst_buffer = *buffer_ptr;
			buffer_update_info.update_size = data.size();

			resource_loader.BeginBufferUpdate(buffer_update_info);
			util::Copy(buffer_update_info.map_info.data, data.data(), data.size(), std::execution::par);
			buffer_update_info.regions[buffer_update_info.region_count++] = { 0ull, 0ull, data.size() };
			resource_loader.EndBufferUpdate(buffer_update_info);
		}



		InterleavedGeometryBuffer::InterleavedGeometryBuffer(edge::gfx::ResourceLoader& resource_loader, const GeometryBufferCreateInfo& create_info) :
			GeometryBuffer{ resource_loader }
		{
			uint32_t offset{ 0u };

			for (auto& desc : create_info.attribute_descriptions)
			{
				auto& attr_desc = vertex_layout.attributes[vertex_layout.attribute_count++];
				attr_desc.binding = 0;
				attr_desc.format = desc.format;
				attr_desc.offset = offset;
				attr_desc.location = GetVertexAttributeLocation(desc.attribute_flag);

				offset += GetVertexFormatSizeBytes(desc.format);
			}

			auto& binding = vertex_layout.bindings[vertex_layout.binding_count++];
			binding.binding = 0;
			binding.stride = offset;

			index_data.type = create_info.index_description.type;
		}

		InterleavedGeometryBuffer::~InterleavedGeometryBuffer()
		{
			auto& context = resource_loader.GetContext();
			context.Destroy(vertex_data.buffer);
		}

		void InterleavedGeometryBuffer::Flush()
		{
			EDGE_PROFILE_ZONE_SCOPED;

			GeometryBuffer::Flush();

			if (!vertex_data.data.empty())
			{
				TryFlushBuffer(&vertex_data.buffer, vertex_data.data, edge::gfx::BufferUsageFlag::eVertexBuffer);
				vertex_data.data.clear();
			}
		}

		void InterleavedGeometryBuffer::BindVertexBuffers(edge::gfx::CommandList& command_list)
		{
			const uint32_t stride{ 0u };
			uint64_t offset{ 0ull };

			assert(vertex_data.buffer && "VERTEX BUFFER IS NOT ALLOCATED");
			command_list.BindVertexBuffers(1u, &vertex_data.buffer, &stride, &offset);
		}

		uint64_t InterleavedGeometryBuffer::GetBufferAddress(VertexAttributeFlag flag)
		{
			return vertex_data.buffer->GetAddress();
		}

		VertexRange InterleavedGeometryBuffer::AddMesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices, shlib::Mesh& mesh)
		{
			VertexRange vertex_range{};
			vertex_range.first_index = index_data.count;
			vertex_range.index_count = indices.count;

			vertex_range.first_vertex = vertex_data.count;
			vertex_range.vertex_count = attributes[0].count;

			// TODO: implement vertex merging

			if (!indices.byte_range.empty())
			{
				index_data.data.insert(index_data.data.end(), indices.byte_range.begin(), indices.byte_range.end());
				index_data.count += indices.count;
			}

			assert(false && "NOT SUPPORTED FOR INTERLEAVED FOR NOW");

			return vertex_range;
		}

		void InterleavedGeometryBuffer::AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag)
		{
			// TODO: assert here when stride is not correct
			vertex_data.data.insert(vertex_data.data.end(), static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + count * stride);
			vertex_data.count += count;
		}


		DeinterleavedGeometryBuffer::DeinterleavedGeometryBuffer(edge::gfx::ResourceLoader& resource_loader, const GeometryBufferCreateInfo& create_info) :
			GeometryBuffer{ resource_loader }
		{
			for (auto& desc : create_info.attribute_descriptions)
			{
				auto& attr_desc = vertex_layout.attributes[vertex_layout.attribute_count++];
				attr_desc.binding = vertex_layout.binding_count;
				attr_desc.format = desc.format;
				attr_desc.location = GetVertexAttributeLocation(desc.attribute_flag);

				auto& bind_desc = vertex_layout.bindings[vertex_layout.binding_count++];
				bind_desc.binding = attr_desc.binding;
				bind_desc.stride = GetVertexFormatSizeBytes(desc.format);
			}

			index_data.type = create_info.index_description.type;
		}

		DeinterleavedGeometryBuffer::~DeinterleavedGeometryBuffer()
		{
			auto& context = resource_loader.GetContext();
			for (auto& vert_data : vertex_datas)
			{
				context.Destroy(vert_data.buffer);
			}
		}

		void DeinterleavedGeometryBuffer::Flush()
		{
			EDGE_PROFILE_ZONE_SCOPED;

			GeometryBuffer::Flush();

			for (auto& vertex_data : vertex_datas)
			{
				if (!vertex_data.data.empty())
				{
					TryFlushBuffer(&vertex_data.buffer, vertex_data.data, edge::gfx::BufferUsageFlag::eVertexBuffer);
					vertex_data.data.clear();
				}
			}
		}

		void DeinterleavedGeometryBuffer::BindVertexBuffers(edge::gfx::CommandList& command_list)
		{
			uint32_t buffer_count{ 0u };
			std::array<edge::gfx::Buffer*, k_max_attribure_description> vertex_buffers{};
			std::array<uint32_t, k_max_attribure_description> strides{};
			std::array<uint64_t, k_max_attribure_description> offsets{};

			for (auto& vert_data : vertex_datas)
			{
				if (vert_data.buffer)
				{
					vertex_buffers[buffer_count++] = vert_data.buffer;
				}
			}

			command_list.BindVertexBuffers(buffer_count, vertex_buffers.data(), strides.data(), offsets.data());
		}

		uint64_t DeinterleavedGeometryBuffer::GetBufferAddress(VertexAttributeFlag flag)
		{
			uint32_t index = GetVertexAttributeLocation(flag);
			assert(vertex_datas[index].buffer && "BUFFER IS NOT ALLOCATED");
			return vertex_datas[index].buffer->GetAddress();
		}

		VertexRange DeinterleavedGeometryBuffer::AddMesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices, shlib::Mesh& mesh)
		{
			VertexRange vertex_range{};
			vertex_range.first_index = index_data.count;
			vertex_range.index_count = indices.count;

			vertex_range.first_vertex = vertex_datas[0].count;
			vertex_range.vertex_count = attributes[0].count;

			for (auto& attribute : attributes)
			{
				if (attribute.byte_range.empty())
					continue;

				uint32_t index = GetVertexAttributeLocation(attribute.attribute_flag);
				auto& vert_data = vertex_datas[index];

				// Maximum vertex size
				// position: vec3 - 12 bytes
				// texcoord: vec2 - 8 bytes
				// normal: vec3 - 12 bytes -> u16vec2 - 4 bytes
				// tangent: vec4 - 16 bytes -> u16vec2 - 4 bytes
				// color: vec4 - 16 bytes -> u8vec4 - 4 bytes
				// joints: u16vec4/u8vec4 - 8 bytes/4 bytes
				// weights: vec4 - 16 bytes -> vec3 - 12 bytes
				// Total(before): 12 + 8 + 12 + 16 + 16 + 8|4 + 16 = 84|88
				// Total(after): 12 + 8 + 4 + 4 + 4 + 8|4 + 12 = 48|52

				// TODO: Move it in to mesh compiler
				if (attribute.attribute_flag & VertexAttributeFlag::ePosition)
				{
					mesh.bounds.min = glm::vec3(std::numeric_limits<float>::infinity());
					mesh.bounds.max = -glm::vec3(std::numeric_limits<float>::infinity());

					mesh.bounds_enc = glm::vec3(1.0f);

					auto* begin = reinterpret_cast<const glm::vec3*>(attribute.byte_range.data());
					auto* end = begin + attribute.count;

					for (auto it = begin; it != end; ++it)
					{
						mesh.bounds_enc = glm::max(mesh.bounds_enc, glm::abs(*it));
						mesh.bounds.min = glm::min(mesh.bounds.min, *it);
						mesh.bounds.max = glm::max(mesh.bounds.max, *it);
					}

					mesh.bounds_enc = glm::exp2(glm::ceil(glm::log2(mesh.bounds_enc)));

					//std::vector<glm::i16vec3> encoded{};
					//for (auto it = begin; it != end; ++it)
					//	encoded.emplace_back(shlib::EncodePosition(*it, mesh.bounds_enc));
					//
					//auto* enc_begin = reinterpret_cast<const uint8_t*>(encoded.data());
					//auto* enc_end = enc_begin + attribute.count * sizeof(glm::i16vec3);
					//
					//vert_data.data.insert(vert_data.data.end(), enc_begin, enc_end);
					vert_data.data.insert(vert_data.data.end(), attribute.byte_range.begin(), attribute.byte_range.end());
				}
				else if (attribute.attribute_flag & VertexAttributeFlag::eNormal)
				{
					auto* begin = reinterpret_cast<const glm::vec3*>(attribute.byte_range.data());
					auto* end = begin + attribute.count;

					std::vector<glm::u16vec2> encoded{};
					for (auto it = begin; it != end; ++it)
						encoded.emplace_back(shlib::EncodeNormal(*it));

					auto* enc_begin = reinterpret_cast<const uint8_t*>(encoded.data());
					auto* enc_end = enc_begin + attribute.count * sizeof(glm::u16vec2);
					
					vert_data.data.insert(vert_data.data.end(), enc_begin, enc_end);
				}
				else if (attribute.attribute_flag & VertexAttributeFlag::eTangent)
				{
					auto* begin = reinterpret_cast<const glm::vec4*>(attribute.byte_range.data());
					auto* end = begin + attribute.count;

					std::vector<glm::u16vec2> encoded{};
					for (auto it = begin; it != end; ++it)
						encoded.emplace_back(shlib::EncodeTangent(*it));

					auto* enc_begin = reinterpret_cast<const uint8_t*>(encoded.data());
					auto* enc_end = enc_begin + attribute.count * sizeof(glm::u16vec2);

					vert_data.data.insert(vert_data.data.end(), enc_begin, enc_end);
				}
				else if (attribute.attribute_flag & VertexAttributeFlag::eColor)
				{
					auto* begin = reinterpret_cast<const glm::vec4*>(attribute.byte_range.data());
					auto* end = begin + attribute.count;

					std::vector<glm::u8vec4> encoded{};
					for (auto it = begin; it != end; ++it)
						encoded.emplace_back(shlib::EncodeColor(*it));

					auto* enc_begin = reinterpret_cast<const uint8_t*>(encoded.data());
					auto* enc_end = enc_begin + attribute.count * sizeof(glm::u8vec4);

					vert_data.data.insert(vert_data.data.end(), enc_begin, enc_end);
				}
				else if (attribute.attribute_flag & VertexAttributeFlag::eWeight)
				{
					auto* begin = reinterpret_cast<const glm::vec4*>(attribute.byte_range.data());
					auto* end = begin + attribute.count;

					// Not needed to store 4 floats, because w3 = 1.0f – w0 – w1 – w2
					std::vector<glm::vec3> encoded{};
					for (auto it = begin; it != end; ++it)
						encoded.emplace_back(glm::vec3(*it));

					auto* enc_begin = reinterpret_cast<const uint8_t*>(encoded.data());
					auto* enc_end = enc_begin + attribute.count * sizeof(glm::vec3);

					vert_data.data.insert(vert_data.data.end(), enc_begin, enc_end);
				}
				//else if (attribute.attribute_flag & VertexAttributeFlag::eTexCoord)
				//{
				//	auto* begin = reinterpret_cast<const glm::vec2*>(attribute.byte_range.data());
				//	auto* end = begin + attribute.count;
				//
				//	for (auto it = begin; it != end; ++it)
				//	{
				//		mesh.uv_bounds.min = glm::min(mesh.uv_bounds.min, *it);
				//		mesh.uv_bounds.max = glm::max(mesh.uv_bounds.max, *it);
				//	}
				//
				//	// Round off the minimum and the maximum of the bounding square to the next
				//	// power of 2 if they are not equal to 0
				//	{
				//		// Select a relatively big default square size to avoid log2 NaN
				//		glm::vec2 p = glm::log2(glm::max(glm::max(-mesh.uv_bounds.min, mesh.uv_bounds.max), 1.0f));
				//		glm::vec2 bs = glm::exp2(glm::ceil(p));
				//		mesh.uv_bounds.min = glm::mix(glm::vec2(0.0f), -bs, glm::notEqual(mesh.uv_bounds.min, glm::vec2(0.0f)));
				//		mesh.uv_bounds.max = glm::mix(glm::vec2(0.0f), bs, glm::notEqual(mesh.uv_bounds.max, glm::vec2(0.0f)));
				//	}
				//
				//	std::vector<glm::u16vec2> encoded{};
				//	for (auto it = begin; it != end; ++it)
				//		encoded.emplace_back(shlib::EncodeTexcoord(*it, mesh.uv_bounds));
				//
				//	auto* enc_begin = reinterpret_cast<const uint8_t*>(encoded.data());
				//	auto* enc_end = enc_begin + attribute.count * sizeof(glm::u16vec2);
				//
				//	vert_data.data.insert(vert_data.data.end(), enc_begin, enc_end);
				//}
				else
				{
					vert_data.data.insert(vert_data.data.end(), attribute.byte_range.begin(), attribute.byte_range.end());
				}

				vert_data.count += attribute.count;
			}

			if (!indices.byte_range.empty())
			{
				index_data.data.insert(index_data.data.end(), indices.byte_range.begin(), indices.byte_range.end());
				index_data.count += indices.count;
			}

			return vertex_range;
		}

		void DeinterleavedGeometryBuffer::AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag)
		{
			uint32_t index = GetVertexAttributeLocation(flag);
			auto& vert_data = vertex_datas[index];
			vert_data.data.insert(vert_data.data.end(), static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + count * stride);
			vert_data.count += count;
		}
	}
}