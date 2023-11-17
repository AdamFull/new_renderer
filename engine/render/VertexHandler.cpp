#include "VertexHandler.h"

#include <logger/logger.h>

using namespace engine::render;

CVertexHandler::CVertexHandler(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CVertexHandler::create()
{
	// Create vertex buffer
	FBufferCreateInfo buffer_ci{};
	buffer_ci.flags = BUFFER_TYPE_VERTEX_FLAG_BIT;
	buffer_ci.size = m_vertexBuffer.elementSize;
	buffer_ci.count = m_vertexBuffer.elementCount;
	buffer_ci.upload.data = m_vertexBuffer.data;
	m_vertexBuffer.buffer = m_pRenderContext->createBuffer("scene vertices", buffer_ci);

	// Create index buffer
	buffer_ci.flags = BUFFER_TYPE_VERTEX_FLAG_BIT;
	buffer_ci.size = m_indexBuffer.elementSize;
	buffer_ci.count = m_indexBuffer.elementCount;
	buffer_ci.upload.data = m_indexBuffer.data;
	m_indexBuffer.buffer = m_pRenderContext->createBuffer("scene indices", buffer_ci);
}

// TODO: i can create custom bindings support here
void CVertexHandler::bind(uint32_t queue)
{
	if (m_vertexBuffer.elementCount)
	{
		{
			FVertexBufferBindingInfo binding_info{};
			binding_info.buffers.emplace_back(m_vertexBuffer.buffer);
			binding_info.offsets.emplace_back(0ull);
			m_pRenderContext->bindVertexBuffers(binding_info, queue);
		}

		if (m_indexBuffer.elementCount)
		{
			FIndexBufferBindingInfo binding_info{};
			binding_info.bufferId = m_indexBuffer.buffer;
			binding_info.indexSize = m_indexBuffer.elementSize;
			m_pRenderContext->bindIndexBuffer(binding_info, queue);
		}
	}
}

size_t CVertexHandler::addVertices(const std::vector<FVertexAttributePointer>& attributes_data)
{
	if (m_vertexBuffer.elementSize == 0ull)
	{
		for (const auto& attrib : attributes_data)
			m_vertexBuffer.elementSize += attrib.stride;
	}
	else
	{
		size_t vertex_size{ 0ull };
		for (const auto& attrib : attributes_data)
			vertex_size += attrib.stride;

		log_cerror(vertex_size == m_vertexBuffer.elementSize, "Your vertex size was {}, but now it's {}.", m_vertexBuffer.elementSize, vertex_size);
	}

	auto vertex_count = attributes_data.front().count;
	for (size_t v = 0ull; v < vertex_count; ++v)
	{
		std::vector<uint8_t> temp(m_vertexBuffer.elementSize);

		size_t offset{ 0ull };
		for (const auto& attrib : attributes_data)
		{
			std::memcpy(temp.data() + offset, &attrib.data[v * attrib.stride], attrib.stride);
			offset += attrib.stride;
		}

		m_vertexBuffer.data.insert(m_vertexBuffer.data.end(), temp.begin(), temp.end());
	}

	m_vertexBuffer.elementCount += vertex_count;
	return vertex_count;
}

size_t CVertexHandler::addIndices(const FVertexAttributePointer& indices)
{
	if (m_indexBuffer.elementSize == 0ull)
		m_indexBuffer.elementSize = indices.stride;
	else
		log_cerror(m_indexBuffer.elementSize == indices.stride, "Your index size was {}, but now it's {}.", m_indexBuffer.elementSize, indices.stride);

	std::vector<char> temp(indices.count * m_indexBuffer.elementSize);
	std::memcpy(temp.data(), indices.data, indices.count * m_indexBuffer.elementSize);
	m_indexBuffer.data.insert(m_indexBuffer.data.end(), temp.begin(), temp.end());

	m_indexBuffer.elementCount += indices.count;
	return indices.count;
}

size_t CVertexHandler::getLastVertex()
{
	return m_vertexBuffer.elementCount;
}

size_t CVertexHandler::getLastIndex()
{
	return m_indexBuffer.elementCount;
}