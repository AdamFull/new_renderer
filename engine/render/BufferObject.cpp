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

void CBufferObject::create(const std::string& name, const FBufferCreateInfo& create_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_graph = m_pRenderInterface->getRenderGraph();
	m_bufferId = render_graph->addRenderResource(name, create_info);
	
	auto buffer_view = render_context->getBufferView(m_bufferId); 
	m_elemCount = buffer_view.elementCount;
	m_elemSize = buffer_view.alignedSize / buffer_view.elementCount;
	m_virtualAddress = buffer_view.address;
}

void CBufferObject::resize(const std::string& name, const FBufferCreateInfo& create_info)
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

void CBufferObject::upload(utl::vector<u8>&& data, const utl::vector<FBufferUploadRegion>& regions)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	FBufferUploadInfo upload_info{};
	upload_info.bufferId = m_bufferId;
	upload_info.data = std::move(data);
	upload_info.regions = regions;

	render_context->uploadDataToBuffer(upload_info);
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
	if (size == INVALID_HANDLE)
	{
		std::memcpy(m_pMapped, data, m_elemSize * m_elemCount);
		return;
	}

	std::memcpy(utl::cast<char*>(m_pMapped) + offset, data, size * m_elemSize);
}

void CBufferObject::flush(u64 size, u64 offset) 
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->flushBuffer(m_bufferId, size, offset);
}

void CBufferObject::clear()
{
	std::memset(m_pMapped, 0, m_elemCount * m_elemSize);
}

u64 CBufferObject::size()
{
	return m_elemSize * m_elemCount;
}

u64 CBufferObject::instanceSize()
{
	return m_elemSize;
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

void* CBufferObject::data()
{
	return m_pMapped;
}