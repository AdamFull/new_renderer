#include "GPUVector.h"
#include "BufferObject.h"

using namespace engine::render;

CGPUVector::CGPUVector(IRenderInterface* render_interface)
{
	m_pBuffer = std::make_unique<CBufferObject>(render_interface);
}

CGPUVector::~CGPUVector()
{
	m_pBuffer->unmap();
}

void CGPUVector::resize(u64 new_size)
{
	if (m_pBuffer->count() < new_size)
	{
		realloc(new_size);
		m_size = new_size;
	}
	else
		m_size = m_pBuffer->count();
}

u64 CGPUVector::capacity()
{
	return m_pBuffer->count();
}

const stl::unique_ptr<IBufferObject>& CGPUVector::buffer()
{
	return m_pBuffer;
}

void CGPUVector::alloc(const std::string& name, EBufferTypeFlagBits flag, u64 stride, u64 capacity)
{
	FBufferCreateInfo buffer_ci{};
	buffer_ci.size = stride;
	buffer_ci.count = capacity;
	buffer_ci.flags = m_bufferFlag = flag;

	m_srOriginalName = name;

	m_pBuffer->create(name, buffer_ci);
	m_pBuffer->map();
}

void CGPUVector::realloc(u64 capacity)
{
	std::vector<char> temporary{};
	temporary.resize(m_pBuffer->size());

	const void* mem_begin = m_pBuffer->data();
	std::memcpy(temporary.data(), mem_begin, temporary.size());

	FBufferCreateInfo buffer_ci{};
	buffer_ci.count = capacity;
	buffer_ci.size = m_pBuffer->instanceSize();
	buffer_ci.flags = m_bufferFlag;

	m_pBuffer->resize(m_srOriginalName, buffer_ci);
	m_pBuffer->map();
}

void* CGPUVector::at(u64 index)
{
	auto* mapped = m_pBuffer->data();
	return utl::cast<char*>(mapped) + (index * m_pBuffer->instanceSize());
}

const void* CGPUVector::at(u64 index) const
{
	auto* mapped = m_pBuffer->data();
	return utl::cast<char*>(mapped) + (index * m_pBuffer->instanceSize());
}