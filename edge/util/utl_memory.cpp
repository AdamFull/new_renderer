#include "utl_memory.hpp"

#include <cstdint>
#include <memory>

namespace edge
{
	buffer::buffer(std::size_t size, std::size_t align)
	{
		if (std::align(align, size, memory_, aligned_size_))
		{
			size_ = size;
		}
	}

	buffer::~buffer()
	{
		if (memory_)
		{
			std::free(memory_);
			memory_ = nullptr;
		}
	}

	bool buffer::valid() const
	{
		return (memory_ && size_);
	}

	uint16_t buffer::resize(std::size_t new_size, std::size_t align)
	{
		void* new_memory;
		std::size_t new_aligned_size;
		if (std::align(align, new_size, new_memory, new_aligned_size))
		{
			std::memcpy(new_memory, memory_, aligned_size_);
			std::free(memory_);
			size_ = new_size;
			aligned_size_ = new_aligned_size;

			return error_code::ok;
		}

		return static_cast<uint16_t>(error_code::bad_alloc);
	}

	std::size_t buffer::size() const
	{
		return aligned_size_;
	}

	raw_memory_view buffer::get_raw_memory_view()
	{
		return raw_memory_view(aligned_size_, memory_);
	}
}