#include "RangeGPUVector.h"
#include "BufferObject.h"

#include <memory.hpp>

using namespace engine::render;

CRangeGPUVector::CRangeGPUVector(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CRangeGPUVector::Upload()
{
	utl::ring_memory_pool<1024> temp_pool;

	for (auto& block : m_vBlocks)
	{
		auto* ptr = reinterpret_cast<const char*>(&m_vData[block.range.begin]);
		auto hash = utl::murmur_hash(ptr, block.range.count, 0u);
		if (block.hash != hash)
		{
			block.hash = hash;
			m_vUploadRanges.emplace_back(upload_range{ block.range.begin, block.range.count });
		}
	}

	if (!m_vUploadRanges.empty())
	{
		// Buffer size is too small, need to resize it
		auto& buffer = m_pRenderInterface->GetBufferObject(m_buffer);
		if (buffer->Size() < m_vData.size())
		{
			FBufferCreateInfo buffer_ci{};
			buffer_ci.count = m_vData.size();
			buffer_ci.size = 1u;
			buffer_ci.flags = m_bufferFlag;

			buffer->Resize(m_srOriginalName, buffer_ci);
		}

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

		// Create upload infos
		u64 src_offset{ 0ull };
		utl::vector<u8> upload_data{};
		utl::vector<FBufferUploadRegion> upload_regions{};
		for (const auto& range : merged_ranges)
		{
			auto& region = upload_regions.emplace_back();
			region.srcOffset = src_offset;
			region.dstOffset = range.offset;
			region.count = range.size / m_elemSize;
			region.size = range.size;

			auto range_start = m_vData.begin() + range.offset;
			upload_data.insert(upload_data.end(), range_start, range_start + range.size);

			src_offset += range.size;
		}

		buffer->Upload(std::move(upload_data), upload_regions);

		m_vUploadRanges.clear();
	}

	m_cursor = 0u;
}

void CRangeGPUVector::Resize(u64 new_size)
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

const std::unique_ptr<IBufferObject>& CRangeGPUVector::Buffer()
{
	return m_pRenderInterface->GetBufferObject(m_buffer);
}

void CRangeGPUVector::Allocate(const std::string& name, EBufferUsageFlags flag, u64 stride, u64 capacity)
{
	FBufferCreateInfo buffer_ci{};
	buffer_ci.size = stride;
	buffer_ci.count = capacity;
	buffer_ci.flags = m_bufferFlag = flag;

	m_srOriginalName = name;

	m_buffer = m_pRenderInterface->CreateBufferObject(name, buffer_ci);
	m_elemSize = stride;//m_pBuffer->instanceSize();
}