#include <GeometryBuffer.h>

#include <gfx/Context.h>

namespace engine
{
	namespace render
	{
		GeometryBuffer::GeometryBuffer(gfx::Context& context) :
			context{ context }
		{

		}

		GeometryBuffer::~GeometryBuffer()
		{

		}

		void GeometryBuffer::AddVertexStream(VertexAttributeType type, const uint8_t* data, uint32_t stride, uint32_t count)
		{
			gfx::BufferCreateInfo create_info{};
			create_info.usage = gfx::BufferUsageFlag::eVertexBuffer | gfx::BufferUsageFlag::eStorageBuffer;

			auto& new_vertex_stream = vertex_streams.emplace_back(context.CreateBuffer(create_info));
		}
	}
}