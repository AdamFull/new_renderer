#include <engine/rendering/InterleavedGeometryBuffer.h>

#include <engine/rendering/gfx/Context.h>

namespace engine
{
	namespace render
	{	
		InterleavedGeometryBuffer::InterleavedGeometryBuffer(gfx::Context& context, const GeometryBufferCreateInfo& create_info) :
			context{ context }
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
			context.Destroy(vertex_data.buffer);
			context.Destroy(index_data.buffer);
		}

		void InterleavedGeometryBuffer::Flush()
		{
			if (!vertex_data.buffer)
			{
				gfx::BufferCreateInfo create_info{};
				create_info.usage = gfx::BufferUsageFlag::eVertexBuffer;
				create_info.size = vertex_data.data.size();

				vertex_data.buffer = context.CreateBuffer(create_info);
			}
			else
			{
				// Already allocated.
				// Check enough space

				// Realloca if needed
			}

			// Free upload data
			vertex_data.data.clear();
			index_data.data.clear();
		}

		void InterleavedGeometryBuffer::Bind(gfx::CommandList& command_list)
		{
			const uint32_t stride{ 0u };
			uint64_t offset{ 0ull };

			assert(vertex_data.buffer && "VERTEX BUFFER IS NOT ALLOCATED");
			command_list.BindVertexBuffers(1u, &vertex_data.buffer, &stride, &offset);

			if (index_data.buffer)
			{
				command_list.BindIndexBuffer(index_data.buffer, index_data.type, 0ull);
			}
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
	}
}