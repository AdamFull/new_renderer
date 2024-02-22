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

	m_srName = name;
	m_bufferType = create_info.flags;
}

void CBufferObject::resize(u64 size)
{
	FBufferCreateInfo buffer_ci{};
	buffer_ci.size = m_elemSize;
	buffer_ci.count = size;
	buffer_ci.flags = m_bufferType;

	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->updateBuffer(m_srName, m_bufferId, buffer_ci);

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