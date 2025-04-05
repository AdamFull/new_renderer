#pragma once

#include <algorithm>
#include <execution>
#include <cstdint>
#include <cassert>

namespace edge::utl {
	template <typename _Ty>
	concept Copyable = requires(_Ty a, _Ty b) {
		{ std::copy(std::begin(a), std::end(a), std::begin(b)) };
	};

	inline constexpr void* copy(void* dst, size_t dst_offset, const void* src, size_t src_size, size_t src_offset = 0ull) {
		uint8_t* dst_begin = static_cast<uint8_t*>(dst) + dst_offset;
		const uint8_t* src_begin = static_cast<const uint8_t*>(src) + src_offset;
		const uint8_t* src_end = src_begin + src_size;

		return std::copy(src_begin, src_end, dst_begin);
	}

	inline constexpr void* copy(void* dst, const void* src, size_t size) {
		return copy(dst, 0ull, src, size);
	}

	template <Copyable Container>
	inline constexpr void* copy(void* dst, size_t dst_offset, const Container& src, size_t src_offset = 0ull) {
		return copy(dst, dst_offset, src.data(), src.size() * sizeof(typename Container::value_type), src_offset * sizeof(typename Container::value_type));
	}

	template<typename _T, typename _U>
	inline _T cast(_U other) {
		static_assert(!std::is_same<_T, _U>::value, "Attempting to cast to the same type.");

#ifdef _DEBUG
		if (!other) {
			return nullptr;
		}

		_T result = dynamic_cast<_T>(other);
		assert((result) && "Invalid type conversion.");

		return result;
#else
		return static_cast<_T>(other);
#endif
	}
}