#include "BufferObject.h"

using namespace engine::render;

CBufferObject::CBufferObject(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

CBufferObject::~CBufferObject()
{
	m_pRenderContext->destroyBuffer(m_bufferId);
}

void CBufferObject::create(const std::string& name, const FBufferCreateInfo& create_info)
{
	m_bufferId = m_pRenderContext->createBuffer(name, create_info);
	
	auto buffer_view = m_pRenderContext->getBufferView(m_bufferId);
	m_elemCount = buffer_view.elementCount;
	m_elemSize = buffer_view.alignedSize / buffer_view.elementCount;
}

void CBufferObject::map()
{
	m_pRenderContext->mapBufferMemory(m_bufferId, m_pMapped);
}

void CBufferObject::unmap() 
{
	if (m_pMapped)
	{
		m_pRenderContext->unmapBufferMemory(m_bufferId);
		m_pMapped = nullptr;
	}
}

void CBufferObject::write(void* data, size_t size, size_t offset) 
{
	if (size == ~0ull)
	{
		std::memcpy(m_pMapped, data, m_elemSize * m_elemCount);
		return;
	}

	std::memcpy(static_cast<char*>(m_pMapped) + offset, data, size);
}

void CBufferObject::flush(size_t size, size_t offset) 
{
	m_pRenderContext->flushBuffer(m_bufferId, size, offset);
}

size_t CBufferObject::handle()
{
	return m_bufferId;
}

char* CBufferObject::at(size_t index)
{
	return static_cast<char*>(m_pMapped) + (index * m_elemSize);
}