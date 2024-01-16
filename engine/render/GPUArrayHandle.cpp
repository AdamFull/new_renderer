#include "GPUArrayHandle.h"
#include "BufferObject.h"

using namespace engine::render;

// TODO: synchronize atomic with gpu side atomic

CGPUArrayHandle::~CGPUArrayHandle()
{
	m_pBuffer->unmap();
}

CGPUArrayHandle::CGPUArrayHandle(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;

	m_pBuffer = std::make_unique<CBufferObject>(m_pRenderInterface);
}

void CGPUArrayHandle::create(const std::string& name, u64 stride, u64 count)
{
	FBufferCreateInfo buffer_ci{};
	buffer_ci.flags = EBufferTypeFlagBits::eStorage;
	m_elementSize = buffer_ci.size = stride;
	m_elementCount = buffer_ci.count = count;

	m_pBuffer->create(name, buffer_ci);
	m_elements.resize(m_elementSize * m_elementCount);

	m_pBuffer->map();
}

void CGPUArrayHandle::flush(u64 count, u64 offset)
{
	u64 element_count = m_currentElementCount.load(std::memory_order_relaxed) + 1ull;
	if (count != ~0ull)
		element_count = count;
	else if (count == 0ull)
		return;

	auto* dst = m_pBuffer->at(m_elementSize * offset);
	std::memcpy(dst, &m_elements[m_elementSize * offset], m_elementSize * element_count);
}

void CGPUArrayHandle::sync(u64 count)
{
	auto* src = m_pBuffer->at(0ull);
	std::memcpy(&m_elements[0ull], src, m_elementSize * count);
}

uint8_t* CGPUArrayHandle::at(u64 index)
{
	assert(index * m_elementSize < m_elements.size());
	return &m_elements[index * m_elementSize];
}

const uint8_t* CGPUArrayHandle::at(u64 index) const
{
	assert(index * m_elementSize < m_elements.size());
	return &m_elements[index * m_elementSize];
}

void CGPUArrayHandle::insert(const void* data, u64 index)
{
	m_currentElementCount.store(std::max(index, m_currentElementCount.load(std::memory_order_relaxed)));
	std::memcpy(at(index), data, m_elementSize);
}

uint8_t* CGPUArrayHandle::emplace_back(const void* data)
{
	u64 current_index = m_currentElementCount.load(std::memory_order_relaxed);
	insert(data, current_index);
	m_currentElementCount.fetch_add(1ull, std::memory_order_relaxed);
	return at(current_index);
}

uint8_t* CGPUArrayHandle::back()
{
	u64 current_index = m_currentElementCount.load(std::memory_order_relaxed);
	if (current_index != 0ull)
		return at(current_index - 1ull);

	return nullptr;
}

const uint8_t* CGPUArrayHandle::back() const
{
	u64 current_index = m_currentElementCount.load(std::memory_order_relaxed);
	if (current_index != 0ull)
		return at(current_index);

	return nullptr;
}

void CGPUArrayHandle::clear()
{
	m_currentElementCount.store(0ull, std::memory_order_relaxed);
}

const std::vector<uint8_t>& CGPUArrayHandle::getData() const
{
	return m_elements;
}

const std::unique_ptr<IBufferObject>& CGPUArrayHandle::getBufferObject() const
{
	return m_pBuffer;
}

u64 CGPUArrayHandle::size() const
{
	return m_currentElementCount.load(std::memory_order_relaxed);
}