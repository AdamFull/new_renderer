#include "BufferObject.h"

using namespace engine::render;

CBufferObject::CBufferObject(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

CBufferObject::~CBufferObject()
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->destroyBuffer(m_bufferId);
}

void CBufferObject::create(const stl::string& name, const FBufferCreateInfo& create_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_graph = m_pRenderInterface->getRenderGraph();
	m_bufferId = render_graph->addRenderResource(name, create_info);
	
	auto buffer_view = render_context->getBufferView(m_bufferId);
	m_elemCount = buffer_view.elementCount;
	m_elemSize = buffer_view.alignedSize / buffer_view.elementCount;
	m_virtualAddress = buffer_view.address;
}

void CBufferObject::resize(const stl::string& name, const FBufferCreateInfo& create_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	if (m_pMapped)
		render_context->unmapBufferMemory(m_bufferId);

	render_context->updateBuffer(name, m_bufferId, create_info);

	auto buffer_view = render_context->getBufferView(m_bufferId);
	m_elemCount = buffer_view.elementCount;
	m_elemSize = buffer_view.alignedSize / buffer_view.elementCount;
	m_virtualAddress = buffer_view.address;
}

void CBufferObject::map()
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->mapBufferMemory(m_bufferId, m_pMapped);
}

void CBufferObject::unmap() 
{
	if (m_pMapped)
	{
		auto& render_context = m_pRenderInterface->getRenderContext();
		render_context->unmapBufferMemory(m_bufferId);
		m_pMapped = nullptr;
	}
}

void CBufferObject::write(const void* data, u64 size, u64 offset)
{
	if (size == ~0ull)
	{
		stl::memcpy(m_pMapped, data, m_elemSize * m_elemCount);
		return;
	}

	stl::memcpy(utl::cast<char*>(m_pMapped) + offset, data, size * m_elemSize);
}

void CBufferObject::flush(u64 size, u64 offset) 
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->flushBuffer(m_bufferId, size, offset);
}

void CBufferObject::clear()
{
	stl::memset(m_pMapped, 0, m_elemCount * m_elemSize);
}

u64 CBufferObject::size()
{
	return m_elemSize * m_elemCount;
}

u64 CBufferObject::count()
{
	return m_elemCount;
}

u64 CBufferObject::handle()
{
	return m_bufferId;
}

uint64_t CBufferObject::address()
{
	return m_virtualAddress;
}

char* CBufferObject::at(u64 index)
{
	return utl::cast<char*>(m_pMapped) + (index * m_elemSize);
}