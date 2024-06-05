#include "BufferObject.h"

using namespace engine::render;

CBufferObject::CBufferObject(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

CBufferObject::~CBufferObject()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->DestroyBuffer(m_resourceId);
}

void CBufferObject::Create(const std::string& name, const FBufferCreateInfo& create_info)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	m_resourceId = render_context->CreateBuffer(name, create_info);
	
	auto buffer_view = render_context->GetBufferView(m_resourceId); 
	m_elemCount = buffer_view.elementCount;
	m_elemSize = buffer_view.alignedSize / buffer_view.elementCount;
	m_virtualAddress = buffer_view.address;
}

void CBufferObject::Resize(const std::string& name, const FBufferCreateInfo& create_info)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();

	if (m_pMapped)
		render_context->UnmapBufferMemory(m_resourceId);

	render_context->UpdateBuffer(name, m_resourceId, create_info);

	auto buffer_view = render_context->GetBufferView(m_resourceId);
	m_elemCount = buffer_view.elementCount;
	m_elemSize = buffer_view.alignedSize / buffer_view.elementCount;
	m_virtualAddress = buffer_view.address;
}

void CBufferObject::Upload(utl::vector<u8>&& data, const utl::vector<FBufferUploadRegion>& regions)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();

	FBufferUploadInfo upload_info{};
	upload_info.bufferId = m_resourceId;
	upload_info.data = std::move(data);
	upload_info.regions = regions;

	render_context->UploadDataToBuffer(upload_info);
}

void CBufferObject::Map()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->MapBufferMemory(m_resourceId, m_pMapped);
}

void CBufferObject::Unmap() 
{
	if (m_pMapped)
	{
		auto& render_context = m_pRenderInterface->GetRenderContext();
		render_context->UnmapBufferMemory(m_resourceId);
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
	render_context->FlushBuffer(m_resourceId, size, offset);
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

uint64_t CBufferObject::Address()
{
	return m_virtualAddress;
}

void* CBufferObject::Data()
{
	return m_pMapped;
}