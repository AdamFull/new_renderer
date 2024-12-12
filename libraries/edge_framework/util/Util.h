#pragma once

#include <algorithm>
#include <execution>
#include <cstdint>

namespace engine
{
	namespace util
	{
		template <typename _Ty>
		concept Copyable = requires(_Ty a, _Ty b)
		{
			{ std::copy(std::begin(a), std::end(a), std::begin(b)) };
		};

		template<typename _ExecPolicy = std::execution::sequenced_policy>
		inline constexpr void* Copy(void* dst, size_t dst_offset, const void* src, size_t src_size, size_t src_offset = 0ull, const _ExecPolicy& policy = {})
		{
			uint8_t* dst_begin = static_cast<uint8_t*>(dst) + dst_offset;
			const uint8_t* src_begin = static_cast<const uint8_t*>(src) + src_offset;
			const uint8_t* src_end = src_begin + src_size;
			
			return std::copy(policy, src_begin, src_end, dst_begin);
		}

		template<typename _ExecPolicy = std::execution::sequenced_policy>
		inline constexpr void* Copy(void* dst, const void* src, size_t size, const _ExecPolicy& policy = {})
		{
			return Copy<_ExecPolicy>(dst, 0ull, src, size);
		}

		template <Copyable Container, typename _ExecPolicy = std::execution::sequenced_policy>
		inline constexpr void* Copy(void* dst, size_t dst_offset, const Container& src, size_t src_offset = 0ull, const _ExecPolicy& policy = {})
		{
			return Copy<_ExecPolicy>(dst, dst_offset, src.data(), src.size() * sizeof(typename Container::value_type), src_offset * sizeof(typename Container::value_type), policy);
		}
	}
}