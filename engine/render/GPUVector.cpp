#include "GPUVector.h"
#include "BufferObject.h"

#include <memory.hpp>

using namespace engine::render;

CGPUVector::CGPUVector(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CGPUVector::Upload()
{
	utl::ring_memory_pool<1024> temp_pool;

	if (!m_vUploadRanges.empty())
	{
		// Buffer size is too small, need to resize it
		auto& buffer = m_pRenderInterface->GetBufferObject(m_buffer);
		if (buffer->Size() < m_vData.size())
		{
			FBufferCreateInfo buffer_ci{};
			buffer_ci.count = m_vData.size() / m_elemSize;
			buffer_ci.size = m_elemSize;
			buffer_ci.flags = m_bufferFlag;

			buffer->Resize(m_srOriginalName, buffer_ci);

			// Buffer was resized. We should upload whole data
			m_vUploadRanges.clear();
			m_vUploadRanges.emplace_back(upload_range{ 0ull, m_lastCursor });;
		}

		// Sort upload ranges
		std::sort(m_vUploadRanges.begin(), m_vUploadRanges.end(), [](const upload_range& a, const upload_range& b) { return a.offset < b.offset; });

		// Merge upload ranges
		utl::vector<upload_range> merged_ranges{};
		merged_ranges.emplace_back(m_vUploadRanges.front());

		for (u64 i = 1ull; i < m_vUploadRanges.size(); ++i)
		{
			auto& last_range = merged_ranges.back();
			if (m_vUploadRanges[i].offset == last_range.offset + last_range.size)
				last_range.size = std::max(last_range.size, m_vUploadRanges[i].offset + m_vUploadRanges[i].size - last_range.offset);
			else
				merged_ranges.emplace_back(m_vUploadRanges[i]);
		}

		BufferUpdateInfo upload_info{};
		upload_info.update_size = std::accumulate(merged_ranges.begin(), merged_ranges.end(), 0, [](uint64_t acc, const upload_range& item) { return acc + item.size; });
		buffer->Lock(upload_info);

		// Create upload infos
		u64 src_offset{ 0ull };
		for (const auto& range : merged_ranges)
		{
			auto& region = upload_info.regions.emplace_back();
			region.srcOffset = src_offset;
			region.dstOffset = range.offset;
			region.count = range.size / m_elemSize;
			region.size = range.size;

			uint8_t* range_start = &m_vData[range.offset];
			util::Copy(upload_info.data, src_offset, m_vData.data(), range.size, range.offset);

			src_offset += range.size;
		}

		
		buffer->Unlock(upload_info);

		m_vUploadRanges.clear();
	}

	m_cursor = 0u;
}

void CGPUVector::Resize(u64 new_size)
{
	u64 size = ToBytes(new_size);

	auto& buffer = m_pRenderInterface->GetBufferObject(m_buffer);
	if (buffer->Size() < size)
	{
		FBufferCreateInfo buffer_ci{};
		buffer_ci.count = new_size;
		buffer_ci.size = m_elemSize;
		buffer_ci.flags = m_bufferFlag;

		buffer->Resize(m_srOriginalName, buffer_ci);
	}

	m_vData.resize(size);
}

const std::unique_ptr<IBufferObject>& CGPUVector::Buffer()
{
	return m_pRenderInterface->GetBufferObject(m_buffer);
}

void CGPUVector::Allocate(const std::string& name, EBufferUsageFlags flag, u64 stride, u64 capacity)
{
	FBufferCreateInfo buffer_ci{};
	buffer_ci.size = stride;
	buffer_ci.count = capacity;
	buffer_ci.flags = m_bufferFlag = flag;

	m_srOriginalName = name;

	m_buffer = m_pRenderInterface->CreateBufferObject(name, buffer_ci);
	m_elemSize = stride;//m_pBuffer->instanceSize();
}