#include "BufferObject.h"

using namespace engine::render;

CBufferObject::CBufferObject(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

CBufferObject::~CBufferObject()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->DestroyBuffer(m_bufferId);
}

void CBufferObject::Create(const std::string& name, const FBufferCreateInfo& create_info)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& render_graph = m_pRenderInterface->GetRenderGraph();
	m_bufferId = render_graph->AddRenderResource(name, create_info);
	
	auto buffer_view = render_context->GetBufferView(m_bufferId); 
	m_elemCount = buffer_view.elementCount;
	m_elemSize = buffer_view.alignedSize / buffer_view.elementCount;
	m_virtualAddress = buffer_view.address;
}

void CBufferObject::Resize(const std::string& name, const FBufferCreateInfo& create_info)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();

	if (m_pMapped)
		render_context->UnmapBufferMemory(m_bufferId);

	render_context->UpdateBuffer(name, m_bufferId, create_info);

	auto buffer_view = render_context->GetBufferView(m_bufferId);
	m_elemCount = buffer_view.elementCount;
	m_elemSize = buffer_view.alignedSize / buffer_view.elementCount;
	m_virtualAddress = buffer_view.address;
}

void CBufferObject::Upload(utl::vector<u8>&& data, const utl::vector<FBufferUploadRegion>& regions)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();

	FBufferUploadInfo upload_info{};
	upload_info.bufferId = m_bufferId;
	upload_info.data = std::move(data);
	upload_info.regions = regions;

	render_context->UploadDataToBuffer(upload_info);
}

void CBufferObject::Map()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->MapBufferMemory(m_bufferId, m_pMapped);
}

void CBufferObject::Unmap() 
{
	if (m_pMapped)
	{
		auto& render_context = m_pRenderInterface->GetRenderContext();
		render_context->UnmapBufferMemory(m_bufferId);
		m_pMapped = nullptr;
	}
}

void CBufferObject::Write(const void* data, u64 size, u64 offset)
{
	if (size == INVALID_HANDLE)
	{
		std::memcpy(m_pMapped, data, m_elemSize * m_elemCount);
		return;
	}

	std::memcpy(utl::cast<char*>(m_pMapped) + offset, data, size * m_elemSize);
}

void CBufferObject::Flush(u64 size, u64 offset) 
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->FlushBuffer(m_bufferId, size, offset);
}

void CBufferObject::Clear()
{
	std::memset(m_pMapped, 0, m_elemCount * m_elemSize);
}

u64 CBufferObject::Size()
{
	return m_elemSize * m_elemCount;
}

u64 CBufferObject::InstanceSize()
{
	return m_elemSize;
}

u64 CBufferObject::Count()
{
	return m_elemCount;
}

u64 CBufferObject::Handle()
{
	return m_bufferId;
}

uint64_t CBufferObject::Address()
{
	return m_virtualAddress;
}

void* CBufferObject::Data()
{
	return m_pMapped;
}