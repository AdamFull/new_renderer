#pragma once

#include "utl_hash.hpp"

namespace edge {
	// String conversion
	template<typename _Ty, typename _Traits = std::char_traits<_Ty>, typename _Allocator = std::allocator<_Ty>>
	inline std::string to_utf8(const std::basic_string<_Ty, _Traits, _Allocator>& str) {}

	template<>
	inline std::string to_utf8<wchar_t>(const std::wstring& str) {
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		return converter.to_bytes(str);
	}

	template<>
	inline std::string to_utf8<char16_t>(const std::u16string& str) {
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
		return converter.to_bytes(str);
	}

	template<>
	inline std::string to_utf8<char32_t>(const std::u32string& str) {
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
		return converter.to_bytes(str);
	}

	template<typename _Ty, typename _Traits = std::char_traits<_Ty>, typename _Allocator = std::allocator<_Ty>>
	inline std::basic_string<_Ty, _Traits, _Allocator> from_utf8(const std::string& str) {}

	template<>
	inline std::wstring from_utf8<wchar_t>(const std::string& str) {
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		return converter.from_bytes(str);
	}

	template<>
	inline std::u16string from_utf8<char16_t>(const std::string& str) {
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
		return converter.from_bytes(str);
	}

	template<>
	inline std::u32string from_utf8<char32_t>(const std::string& str) {
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
		return converter.from_bytes(str);
	}

	// https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594
	inline std::string to_base64(const uint8_t* data, size_t size) {
		static constexpr char sEncodingTable[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
		'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
		'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

		size_t out_len = 4 * ((size + 2) / 3);
		std::string ret(out_len, '\0');
		size_t i;
		char* p = const_cast<char*>(ret.c_str());

		for (i = 0; size > 2 && i < size - 2; i += 3)  {
			*p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
			*p++ = sEncodingTable[((data[i] & 0x3) << 4) |
				((int)(data[i + 1] & 0xF0) >> 4)];
			*p++ = sEncodingTable[((data[i + 1] & 0xF) << 2) |
				((int)(data[i + 2] & 0xC0) >> 6)];
			*p++ = sEncodingTable[data[i + 2] & 0x3F];
		}

		if (i < size)  {
			*p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
			if (i == (size - 1))  {
				*p++ = sEncodingTable[((data[i] & 0x3) << 4)];
				*p++ = '=';
			}
			else {
				*p++ = sEncodingTable[((data[i] & 0x3) << 4) |
					((int)(data[i + 1] & 0xF0) >> 4)];
				*p++ = sEncodingTable[((data[i + 1] & 0xF) << 2)];
			}
			*p++ = '=';
		}

		return ret;
	}

	inline std::vector<uint8_t> from_base64(const std::string& data) {
		static constexpr unsigned char kDecodingTable[] = {
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57,
		58, 59, 60, 61, 64, 64, 64, 64, 64, 64, 64, 0,  1,  2,  3,  4,  5,  6,
		7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
		25, 64, 64, 64, 64, 64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
		37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64 };

		size_t in_len = data.size();
		if (in_len % 4 != 0)
			return {};

		size_t out_len = in_len / 4 * 3;
		if (in_len >= 1 && data[in_len - 1] == '=')
			out_len--;
		if (in_len >= 2 && data[in_len - 2] == '=')
			out_len--;

		std::vector<uint8_t> out;
		out.resize(out_len);

		for (size_t i = 0, j = 0; i < in_len;)  {
			uint32_t a = data[i] == '='
				? 0 & i++
				: kDecodingTable[static_cast<int>(data[i++])];
			uint32_t b = data[i] == '='
				? 0 & i++
				: kDecodingTable[static_cast<int>(data[i++])];
			uint32_t c = data[i] == '='
				? 0 & i++
				: kDecodingTable[static_cast<int>(data[i++])];
			uint32_t d = data[i] == '='
				? 0 & i++
				: kDecodingTable[static_cast<int>(data[i++])];

			uint32_t triple =
				(a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

			if (j < out_len)
				out[j++] = (triple >> 2 * 8) & 0xFF;
			if (j < out_len)
				out[j++] = (triple >> 1 * 8) & 0xFF;
			if (j < out_len)
				out[j++] = (triple >> 0 * 8) & 0xFF;
		}

		return out;
	}

	namespace impl {
		/// Base declaration of our constexpr string_view concatenation helper
		template <std::string_view const&, typename, std::string_view const&, typename>
		struct concat;

		/// Specialisation to yield indices for each char in both provided string_views,
		/// allows us flatten them into a single char array
		template <std::string_view const& S1,
			std::size_t... I1,
			std::string_view const& S2,
			std::size_t... I2>
		struct concat<S1, std::index_sequence<I1...>, S2, std::index_sequence<I2...>> {
			static constexpr const char value[]{ S1[I1]..., S2[I2]..., 0 };
		};
	} // namespace impl

	/// Base definition for compile time joining of strings
	template <std::string_view const&...> struct join;

	/// When no strings are given, provide an empty literal
	template <>
	struct join<> {
		static constexpr std::string_view value = "";
	};

	/// Base case for recursion where we reach a pair of strings, we concatenate
	/// them to produce a new constexpr string
	template <std::string_view const& S1, std::string_view const& S2>
	struct join<S1, S2> {
		static constexpr std::string_view value =
			impl::concat<S1,
			std::make_index_sequence<S1.size()>,
			S2,
			std::make_index_sequence<S2.size()>>::value;
	};

	/// Main recursive definition for constexpr joining, pass the tail down to our
	/// base case specialisation
	template <std::string_view const& S, std::string_view const&... Rest>
	struct join<S, Rest...> {
		static constexpr std::string_view value =
			join<S, join<Rest...>::value>::value;
	};

	/// Join constexpr string_views to produce another constexpr string_view
	template <std::string_view const&... Strs>
	static constexpr auto join_v = join<Strs...>::value;


	template<typename _Ty, typename _HashType>
	class id_string {
	public:
		using value_type = std::basic_string<_Ty, std::char_traits<_Ty>, std::allocator<_Ty>>;
		using hash_type = _HashType;

		id_string() noexcept : _hash(0) {}

		id_string(const value_type& str)
			: _hash(compute_hash(str)) {
			_store_string(_hash, str);
		}

		id_string(const value_type& str, hash_type hash)
			: _hash(hash) {
			_store_string(_hash, str);
		}

		// Copy constructor
		id_string(const id_string& other)
			: _hash(other._hash) {
		}

		// Assignment operator
		id_string& operator=(const id_string& other) {
			if (this != &other && _hash != other._hash && _hash != 0) {
				_hash = other._hash;
				_store_string(_hash, other.c_str());
			}
			return *this;
		}

		const _Ty* c_str() const {
			auto it = _string_pool().find(_hash);
			if (it != _string_pool().end())
				return it->second.c_str();

			return nullptr;
		}

		const value_type& str() const {
			static value_type empty{ "" };

			auto it = _string_pool().find(_hash);
			if (it != _string_pool().end())
				return it->second;

			return empty;
		}

		hash_type hash() const  {
			return _hash;
		}

		bool valid() const {
			return _hash != 0;
		}

	private:
		hash_type _hash;

		static hash_type compute_hash(const value_type& str) {
			if constexpr (sizeof(hash_type) == sizeof(uint32_t)) {
				return murmur_hash((char*)str.data(), str.size() * sizeof(_Ty), 0);
			}
			else if constexpr (sizeof(hash_type) == sizeof(uint64_t)) {
				return murmur_hash_64((char*)str.data(), str.size() * sizeof(_Ty), 0);
			}
		}

		static void _store_string(hash_type hash, const value_type& str) {
			_string_pool()[hash] = str;
		}

		static std::unordered_map<hash_type, std::string>& _string_pool() {
			static std::unordered_map<hash_type, value_type> pool;
			return pool;
		}
	};

	using id_string32 = id_string<char, uint32_t>;
	using id_string64 = id_string<char, uint64_t>;

	using id_wstring32 = id_string<wchar_t, uint32_t>;
	using id_wstring64 = id_string<wchar_t, uint64_t>;

	using id_u16string32 = id_string<char16_t, uint32_t>;
	using id_u16string64 = id_string<char16_t, uint64_t>;

	using id_u32string32 = id_string<char32_t, uint32_t>;
	using id_u32string64 = id_string<char32_t, uint64_t>;

	//template<typename _Ty, typename _HashType>
	//class id_string_view
	//{
	//public:
	//	using value_type = std::basic_string_view<_Ty>;
	//	using hash_type = _HashType;
	//
	//	constexpr id_string_view() noexcept : _hash(0) {}
	//
	//	constexpr id_string_view(value_type str)
	//		: _hash(compute_hash(str))
	//	{
	//		_string = str;
	//	}
	//
	//	constexpr id_string_view(const _Ty* str)
	//		: _hash(compute_hash(str))
	//	{
	//		_string = value_type(str);
	//	}
	//
	//	constexpr id_string_view& operator=(const id_string_view& other)
	//	{
	//		if (this != &other && _hash != other._hash && _hash != 0)
	//		{
	//			_hash = other._hash;
	//			_string = other._string;
	//		}
	//		return *this;
	//	}
	//
	//	id_string_view& operator=(const id_string<_Ty, _HashType>& other) 
	//	{
	//		if (this != &other && _hash != other._hash && _hash != 0)
	//		{
	//			_hash = other._hash;
	//			_string = other.c_str();
	//		}
	//		return *this;
	//	}
	//
	//	inline constexpr const _Ty* c_str() noexcept const
	//	{
	//		return _string.data();
	//	}
	//
	//	inline constexpr const value_type str() noexcept const
	//	{
	//		return _string;
	//	}
	//
	//	inline constexpr hash_type hash() noexcept const
	//	{
	//		return _hash;
	//	}
	//
	//	inline constexpr bool valid() noexcept const
	//	{
	//		return _hash != 0;
	//	}
	//private:
	//	hash_type _hash;
	//	value_type _string;
	//};
}