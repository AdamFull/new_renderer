#include <engine/rendering/DeinterleavedGeometryBuffer.h>

#include <engine/rendering/gfx/Context.h>

namespace engine
{
	namespace render
	{
		DeinterleavedGeometryBuffer::DeinterleavedGeometryBuffer(gfx::Context& context, const GeometryBufferCreateInfo& create_info) :
			context{ context }
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
			for (auto& vert_data : vertex_datas)
			{
				context.Destroy(vert_data.buffer);
			}

			context.Destroy(index_data.buffer);
		}

		void DeinterleavedGeometryBuffer::Bind(gfx::CommandList& command_list)
		{
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

			if (index_data.buffer)
			{
				command_list.BindIndexBuffer(index_data.buffer, index_data.type, 0ull);
			}
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