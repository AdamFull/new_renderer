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
	// TODO: create buffers
}

size_t CVertexHandler::addVertices(const std::vector<FVertexAttributePointer>& attributes_data)
{
	if (m_vertexSize == 0ull)
	{
		for (const auto& attrib : attributes_data)
			m_vertexSize += attrib.stride;
	}
	else
	{
		size_t vertex_size{ 0ull };
		for (const auto& attrib : attributes_data)
			vertex_size += attrib.stride;

		log_cerror(vertex_size == m_vertexSize, "Your vertex size was {}, but now it's {}.", m_vertexSize, vertex_size);
	}

	auto vertex_count = attributes_data.front().count;
	for (size_t v = 0ull; v < vertex_count; ++v)
	{
		std::vector<uint8_t> temp(m_vertexSize);

		size_t offset{ 0ull };
		for (const auto& attrib : attributes_data)
		{
			std::memcpy(temp.data() + offset, &attrib.data[v * attrib.stride], attrib.stride);
			offset += attrib.stride;
		}

		m_vertices.insert(m_vertices.end(), temp.begin(), temp.end());
	}

	m_lastVertex += vertex_count;
	return vertex_count;
}

size_t CVertexHandler::addIndices(const FVertexAttributePointer& indices)
{
	if (m_indexSize == 0ull)
		m_indexSize = indices.stride;
	else
		log_cerror(m_indexSize == indices.stride, "Your index size was {}, but now it's {}.", m_indexSize, indices.stride);

	std::vector<char> temp(indices.count * m_indexSize);
	std::memcpy(temp.data(), indices.data, indices.count * m_indexSize);
	m_indices.insert(m_indices.end(), temp.begin(), temp.end());

	m_lastIndex += indices.count;
	return indices.count;
}

size_t CVertexHandler::getLastVertex()
{
	return m_lastVertex;
}

size_t CVertexHandler::getLastIndex()
{
	return m_lastIndex;
}