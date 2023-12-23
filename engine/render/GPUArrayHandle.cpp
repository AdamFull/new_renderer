#include "GPUArrayHandle.h"
#include "BufferObject.h"

using namespace engine::render;

CGPUArrayHandle::~CGPUArrayHandle()
{
	m_pBuffer->unmap();
}

CGPUArrayHandle::CGPUArrayHandle(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;

	m_pBuffer = std::make_unique<CBufferObject>(m_pRenderInterface, m_pRenderContext);
}

void CGPUArrayHandle::create(const std::string& name, size_t stride, size_t count)
{
	FBufferCreateInfo buffer_ci{};
	buffer_ci.flags = BUFFER_TYPE_STORAGE_FLAG_BIT;
	m_elementSize = buffer_ci.size = stride;
	m_elementCount = buffer_ci.count = count;

	m_pBuffer->create(name, buffer_ci);
	m_elements.resize(stride * count);

	m_pBuffer->map();
}

void CGPUArrayHandle::flush(size_t count, size_t offset)
{
	size_t element_count{ m_lastInsertionIndex + 1ull };
	if (count != ~0ull)
		element_count = count;
	else if (count == 0ull)
		return;

	auto* dst = m_pBuffer->at(m_elementSize * offset);
	std::memcpy(dst, &m_elements[m_elementSize * offset], m_elementSize * element_count);
}

uint8_t* CGPUArrayHandle::at(size_t index)
{
	assert(index * m_elementSize < m_elements.size());
	return &m_elements[index * m_elementSize];
}

const uint8_t* CGPUArrayHandle::at(size_t index) const
{
	assert(index * m_elementSize < m_elements.size());
	return &m_elements[index * m_elementSize];
}

void CGPUArrayHandle::insert(const void* data, size_t index)
{
	m_lastInsertionIndex = std::max(m_lastInsertionIndex, index);
	std::memcpy(at(index), data, m_elementSize);
}

uint8_t* CGPUArrayHandle::emplace_back(const void* data)
{
	size_t current_index = m_currentElementCount.load(std::memory_order_relaxed);
	insert(data, current_index);
	m_currentElementCount.fetch_add(1ull, std::memory_order_relaxed);
	return at(current_index);
}

uint8_t* CGPUArrayHandle::back()
{
	size_t current_index = m_currentElementCount.load(std::memory_order_relaxed);
	if (current_index != 0ull)
		return at(current_index - 1ull);

	return nullptr;
}

const uint8_t* CGPUArrayHandle::back() const
{
	size_t current_index = m_currentElementCount.load(std::memory_order_relaxed);
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

size_t CGPUArrayHandle::getLastInsertionIndex()
{
	return m_lastInsertionIndex;
}