#include "BufferObject.h"

using namespace engine::render;

CBufferObject::CBufferObject(IRenderInterface* render_interface)
{
	auto& render_context = render_interface->GetRenderContext();
	m_pRenderContext = render_context.get();
}

CBufferObject::~CBufferObject()
{
	m_pRenderContext->DestroyBuffer(m_bufferId);
}

void CBufferObject::Create(const std::string& name, const FBufferCreateInfo& create_info)
{
	m_bufferId = m_pRenderContext->CreateBuffer(name, create_info);
	
	auto buffer_view = m_pRenderContext->GetBufferView(m_bufferId);
	m_elemCount = buffer_view.elementCount;
	m_elemSize = buffer_view.alignedSize / buffer_view.elementCount;
	m_virtualAddress = buffer_view.address;
}

void CBufferObject::Resize(const std::string& name, const FBufferCreateInfo& create_info)
{
	if (m_pMapped)
		m_pRenderContext->UnmapBufferMemory(m_bufferId);

	m_pRenderContext->UpdateBuffer(name, m_bufferId, create_info);

	auto buffer_view = m_pRenderContext->GetBufferView(m_bufferId);
	m_elemCount = buffer_view.elementCount;
	m_elemSize = buffer_view.alignedSize / buffer_view.elementCount;
	m_virtualAddress = buffer_view.address;
}

void CBufferObject::Upload(utl::vector<u8>&& data, const utl::vector<FBufferUploadRegion>& regions)
{
	FBufferUploadInfo upload_info{};
	upload_info.bufferId = m_bufferId;
	upload_info.data = std::move(data);
	upload_info.regions = regions;

	m_pRenderContext->UploadDataToBuffer(upload_info);
}

void CBufferObject::Map()
{
	m_pRenderContext->MapBufferMemory(m_bufferId, m_pMapped);
}

void CBufferObject::Unmap() 
{
	if (m_pMapped)
	{
		m_pRenderContext->UnmapBufferMemory(m_bufferId);
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
	m_pRenderContext->FlushBuffer(m_bufferId, size, offset);
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

FAPIHandle CBufferObject::Handle()
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