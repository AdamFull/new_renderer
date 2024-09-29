#pragma once

#include <algorithm>
#include <execution>
#include <cstdint>

namespace engine
{
	namespace util
	{
		template<typename _ExecPolicy = std::execution::sequenced_policy>
		inline constexpr void* Copy(void* dst, const void* src, size_t size, size_t offset = 0ull, const _ExecPolicy& policy = {})
		{
			uint8_t* dst_begin = static_cast<uint8_t*>(dst) + offset;
			const uint8_t* src_begin = static_cast<const uint8_t*>(src);
			const uint8_t* src_end = src_begin + size;
#if 1
			std::copy(policy, src_begin, src_end, dst_begin);
#else
			std::memcpy(dst_begin, src_begin, size);
#endif

			return dst;
		}
	}
}