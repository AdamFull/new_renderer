#include "VertexHandler.h"

#include <logger/logger.h>

using namespace engine::render;

uint32_t get_attribute_format_by_type(EVertexAttributeType type)
{
	switch (type)
	{
	case EVertexAttributeType::ePosition: return FORMAT_RGB32_SFLOAT; break;
	case EVertexAttributeType::eColor: return FORMAT_RGB32_SFLOAT; break;
	case EVertexAttributeType::eTexCoord: return FORMAT_RG32_SFLOAT; break;
	case EVertexAttributeType::eNormal: return FORMAT_RGB32_SFLOAT; break;
	case EVertexAttributeType::eTangent: return FORMAT_RGBA32_SFLOAT; break;
	case EVertexAttributeType::eJoint: return FORMAT_RGBA16_UNORM; break;
	case EVertexAttributeType::eWeight: return FORMAT_RGBA32_SFLOAT; break;
	}
	return 0;
}

CVertexHandler::CVertexHandler(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

// TODO: destroy buffers
CVertexHandler::~CVertexHandler()
{

}


// NOTE: current implementation work only with interleaved vertex
void CVertexHandler::create(const FVertexHandlerCreateInfo& create_info)
{
	for (const auto& desc : create_info.descriptions)
	{
		auto& attrib = m_declaration.attributeDescriptions.emplace_back();
		attrib.binding = 0u;
		attrib.location = static_cast<uint32_t>(desc.type);
		auto format = get_attribute_format_by_type(desc.type);
		attrib.format = m_pRenderContext->getApiFormat(format);
		attrib.offset = m_vertexBuffer.elementSize;

		m_vertexBuffer.elementSize += desc.size;
	}

	auto& desc = m_declaration.bindingDescriptions.emplace_back();
	desc.binding = 0u;
	desc.stride = m_vertexBuffer.elementSize;

	m_indexBuffer.elementSize = create_info.indexSize;
}

void CVertexHandler::allocate()
{
	// Create vertex buffer
	FBufferCreateInfo buffer_ci{};
	buffer_ci.flags = BUFFER_TYPE_VERTEX_FLAG_BIT;
	buffer_ci.size = m_vertexBuffer.elementSize;
	buffer_ci.count = m_vertexBuffer.elementCount;
	buffer_ci.upload.data = m_vertexBuffer.data;
	m_vertexBuffer.buffer = m_pRenderContext->createBuffer("scene vertices", buffer_ci);

	// Create index buffer
	buffer_ci.flags = BUFFER_TYPE_INDEX_FLAG_BIT;
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

const FVertexDeclarationInfo& CVertexHandler::getVertexDeclaration() const
{
	return m_declaration;
}