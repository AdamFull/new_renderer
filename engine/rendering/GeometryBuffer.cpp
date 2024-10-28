#include <engine/rendering/GeometryBuffer.h>

#include <engine/rendering/gfx/Context.h>

#include <util/Util.h>

#include <hash.hpp>

namespace engine
{
	namespace render
	{
		inline uint32_t GetVertexFormatSizeBytes(gfx::Format format)
		{
			switch (format)
			{
			case gfx::Format::eR8G8B8A8Unorm: return 4u; break;
			case gfx::Format::eR32G32Sfloat:
			case gfx::Format::eR16G16B16A16Unorm: return 8u; break;
			case gfx::Format::eR32G32B32Sfloat: return 12u; break;
			case gfx::Format::eR32G32B32A32Sfloat: return 16u; break;
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



		GeometryBuffer::GeometryBuffer(gfx::ResourceContext& resource_context) :
			resource_context{ resource_context }
		{
		}

		GeometryBuffer::~GeometryBuffer()
		{
			auto& context = resource_context.GetContext();
			context.Destroy(index_data.buffer);
		}

		void GeometryBuffer::Flush()
		{
			TryFlushBuffer(&index_data.buffer, index_data.data, gfx::BufferUsageFlag::eIndexBuffer);
			index_data.data.clear();
		}

		void GeometryBuffer::Bind(gfx::CommandList& command_list)
		{
			if (index_data.buffer)
			{
				command_list.BindIndexBuffer(index_data.buffer, index_data.type, 0ull);
			}
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

		gfx::VertexLayout const& GeometryBuffer::GetVertexLayout() const
		{
			return vertex_layout;
		}

		void GeometryBuffer::TryFlushBuffer(gfx::Buffer** buffer_ptr, const std::vector<uint8_t>& data, gfx::BufferUsageFlags usage)
		{
			gfx::Context& context = resource_context.GetContext();

			gfx::Buffer* buffer = *buffer_ptr;
			if (!buffer)
			{
				gfx::BufferCreateInfo create_info{};
				create_info.size = data.size();
				create_info.count = 1ull;
				create_info.usage = usage;

				buffer = context.CreateBuffer(create_info);
			}

			if (buffer->GetSize() < data.size())
			{
				context.Destroy(buffer);

				gfx::BufferCreateInfo create_info{};
				create_info.size = data.size();
				create_info.count = 1ull;
				create_info.usage = usage;

				buffer = context.CreateBuffer(create_info);
			}

			gfx::BufferUpdateInfo buffer_update_info{};
			buffer_update_info.dst_buffer = buffer;
			buffer_update_info.update_size = data.size();

			resource_context.BeginBufferUpdate(buffer_update_info);
			util::Copy(buffer_update_info.map_info.data, data.data(), data.size(), std::execution::par);
			buffer_update_info.regions[buffer_update_info.region_count++] = { 0ull, 0ull, data.size() };
			resource_context.EndBufferUpdate(buffer_update_info);
		}



		InterleavedGeometryBuffer::InterleavedGeometryBuffer(gfx::ResourceContext& resource_context, const GeometryBufferCreateInfo& create_info) :
			GeometryBuffer{ resource_context }
		{
			uint32_t offset{ 0u };

			for (auto& desc : create_info.attribute_descriptions)
			{
				auto& attr_desc = vertex_layout.attributes[vertex_layout.attribute_count];
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
			auto& context = resource_context.GetContext();
			context.Destroy(vertex_data.buffer);
		}

		void InterleavedGeometryBuffer::Flush()
		{
			GeometryBuffer::Flush();

			if (!vertex_data.data.empty())
			{
				TryFlushBuffer(&vertex_data.buffer, vertex_data.data, gfx::BufferUsageFlag::eVertexBuffer);
				vertex_data.data.clear();
			}
		}

		void InterleavedGeometryBuffer::Bind(gfx::CommandList& command_list)
		{
			GeometryBuffer::Bind(command_list);

			const uint32_t stride{ 0u };
			uint64_t offset{ 0ull };

			assert(vertex_data.buffer && "VERTEX BUFFER IS NOT ALLOCATED");
			command_list.BindVertexBuffers(1u, &vertex_data.buffer, &stride, &offset);
		}

		uint64_t InterleavedGeometryBuffer::GetBufferAddress(VertexAttributeFlag flag)
		{
			return vertex_data.buffer->GetAddress();
		}

		void InterleavedGeometryBuffer::AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag)
		{
			// TODO: assert here when stride is not correct
			vertex_data.data.insert(vertex_data.data.end(), static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + count * stride);
			vertex_data.count += count;
		}


		DeinterleavedGeometryBuffer::DeinterleavedGeometryBuffer(gfx::ResourceContext& resource_context, const GeometryBufferCreateInfo& create_info) :
			GeometryBuffer{ resource_context }
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
			auto& context = resource_context.GetContext();
			for (auto& vert_data : vertex_datas)
			{
				context.Destroy(vert_data.buffer);
			}
		}

		void DeinterleavedGeometryBuffer::Flush()
		{
			GeometryBuffer::Flush();

			for (auto& vertex_data : vertex_datas)
			{
				if (!vertex_data.data.empty())
				{
					TryFlushBuffer(&vertex_data.buffer, vertex_data.data, gfx::BufferUsageFlag::eVertexBuffer);
					vertex_data.data.clear();
				}
			}
		}

		void DeinterleavedGeometryBuffer::Bind(gfx::CommandList& command_list)
		{
			GeometryBuffer::Bind(command_list);

			uint32_t buffer_count{ 0u };
			std::array<gfx::Buffer*, k_max_attribure_description> vertex_buffers{};
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

		void DeinterleavedGeometryBuffer::AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag)
		{
			uint32_t index = GetVertexAttributeLocation(flag);
			auto& vert_data = vertex_datas[index];
			vert_data.data.insert(vert_data.data.end(), static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + count * stride);
			vert_data.count += count;
		}
	}
}