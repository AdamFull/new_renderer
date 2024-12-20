#pragma once

#include <string>
#include <string_view>

namespace edge
{
	constexpr inline size_t strlen(const char* str) noexcept
	{
		size_t len = 0;
		while (str[len] != '\0')
			++len;

		return len;
	}

	inline constexpr void hash_combine(uint32_t hash, uint32_t& seed)
	{
		seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	namespace murmur32
	{
		inline constexpr uint32_t block(const char* p, unsigned i)
		{
			const uint32_t block =
				static_cast<uint32_t>(p[0 + i * 4]) << 0 |
				static_cast<uint32_t>(p[1 + i * 4]) << 8 |
				static_cast<uint32_t>(p[2 + i * 4]) << 16 |
				static_cast<uint32_t>(p[3 + i * 4]) << 24;
			return block;
		}

		inline constexpr uint32_t rotl32(uint32_t x, int8_t r) 
		{
			return (x << r) | (x >> (32 - r));
		}

		inline constexpr uint32_t fmix32(uint32_t h) 
		{
			h ^= h >> 16;
			h *= 0x85ebca6b;
			h ^= h >> 13;
			h *= 0xc2b2ae35;
			h ^= h >> 16;
			return h;
		}
	}

	inline constexpr uint32_t murmur_hash(const char* key, uint32_t len, uint32_t seed)
	{
		constexpr const uint32_t c1 = 0xcc9e2d51;
		constexpr const int32_t c2 = 0x1b873593;

		const uint32_t nblocks = len / 4u;

		uint32_t h1 = seed;
		
		//----------
		// body
		for (int i = 0; i < nblocks; ++i) 
		{
			uint32_t k1 = murmur32::block(key, i);

			k1 *= c1;
			k1 = murmur32::rotl32(k1, 15);
			k1 *= c2;

			h1 ^= k1;
			h1 = murmur32::rotl32(h1, 13);
			h1 = h1 * 5 + 0xe6546b64;
		}
		//----------
		// tail

		uint32_t k1 = 0;

		// The "tail" of key are the bytes that do not fit into any block,
		// len%4 is the size of the tail and 
		// 	(tail_start + i)
		// returns the i:th tail byte.
		const uint32_t tail_start = len - (len % 4);
		switch (len & 3) {
		case 3: k1 ^= key[tail_start + 2] << 16;
		case 2: k1 ^= key[tail_start + 1] << 8;
		case 1: k1 ^= key[tail_start + 0];
			k1 *= c1; k1 = murmur32::rotl32(k1, 15);
			k1 *= c2; h1 ^= k1;
		};

		//----------
		// finalization

		h1 ^= len;

		h1 = murmur32::fmix32(h1);

		return h1;
	}

	inline constexpr uint32_t murmur_hash(const char* str, uint32_t seed = 0u)
	{
		return murmur_hash(str, strlen(str), seed);
	}

	template<typename _Ty, typename _Traits = std::char_traits<_Ty>, typename _Allocator = std::allocator<_Ty>>
	inline uint32_t murmur_hash(const std::basic_string<_Ty, _Traits, _Allocator>& str, uint32_t seed = 0u)
	{
		return murmur_hash(str.c_str(), str.size(), seed);
	}

	template<typename _Ty>
	inline constexpr uint32_t murmur_hash(std::basic_string_view<_Ty> str, uint32_t seed = 0u)
	{
		return murmur_hash(str.data(), str.size(), seed);
	}

	template<typename _Ty>
	inline uint32_t murmur_hash(const _Ty& value, uint32_t seed = 0u)
	{
		return murmur_hash(reinterpret_cast<const char*>(&value), sizeof(_Ty), seed);
	}

	namespace murmur64
	{
		inline constexpr uint64_t block(const char* p, uint64_t i)
		{
			const uint64_t block =
				static_cast<uint64_t>(p[0 + i * 8]) << 0 |
				static_cast<uint64_t>(p[1 + i * 8]) << 8 |
				static_cast<uint64_t>(p[2 + i * 8]) << 16 |
				static_cast<uint64_t>(p[3 + i * 8]) << 24 |
				static_cast<uint64_t>(p[4 + i * 8]) << 32 |
				static_cast<uint64_t>(p[5 + i * 8]) << 40 |
				static_cast<uint64_t>(p[6 + i * 8]) << 48 |
				static_cast<uint64_t>(p[7 + i * 8]) << 56;
			return block;
		}

		inline constexpr uint64_t rotl64(uint64_t x, int8_t r)
		{
			return (x << r) | (x >> (64 - r));
		}

		inline constexpr uint64_t fmix64(uint64_t k)
		{
			k ^= k >> 33;
			k *= 0xff51afd7ed558ccdull;
			k ^= k >> 33;
			k *= 0xc4ceb9fe1a85ec53ull;
			k ^= k >> 33;

			return k;
		}
	}

	inline constexpr uint64_t murmur_hash_64(const char* key, uint64_t len, uint64_t seed) noexcept
	{
		constexpr const uint64_t c1 = 0x87c37b91114253d5ULL;
		constexpr const uint64_t c2 = 0x4cf5ad432745937fULL;

		uint64_t h1 = seed;

		const uint64_t nblocks = len / 16ull;

		for (uint64_t i = 0; i < nblocks; ++i)
		{
			uint64_t k1 = murmur64::block(key, i * 2);
			uint64_t k2 = murmur64::block(key, i * 2 + 1);

			k1 *= c1;
			k1 = murmur64::rotl64(k1, 31);
			k1 *= c2;
			h1 ^= k1;

			h1 = murmur64::rotl64(h1, 27);
			h1 = h1 * 5 + 0x52dce729;

			k2 *= c2;
			k2 = murmur64::rotl64(k2, 33);
			k2 *= c1;
			h1 ^= k2;

			h1 = murmur64::rotl64(h1, 31);
			h1 = h1 * 5 + 0x38495ab5;
		}

		const uint64_t tail_start = nblocks * 16ull;
		uint64_t k1 = 0;
		uint64_t k2 = 0;

		switch (len & 15) {
		case 15: k2 ^= uint64_t(key[tail_start + 14]) << 48;
		case 14: k2 ^= uint64_t(key[tail_start + 13]) << 40;
		case 13: k2 ^= uint64_t(key[tail_start + 12]) << 32;
		case 12: k2 ^= uint64_t(key[tail_start + 11]) << 24;
		case 11: k2 ^= uint64_t(key[tail_start + 10]) << 16;
		case 10: k2 ^= uint64_t(key[tail_start + 9]) << 8;
		case 9:  k2 ^= uint64_t(key[tail_start + 8]);
			k2 *= c2;
			k2 = murmur64::rotl64(k2, 33);
			k2 *= c1;
			h1 ^= k2;

		case 8:  k1 ^= uint64_t(key[tail_start + 7]) << 56;
		case 7:  k1 ^= uint64_t(key[tail_start + 6]) << 48;
		case 6:  k1 ^= uint64_t(key[tail_start + 5]) << 40;
		case 5:  k1 ^= uint64_t(key[tail_start + 4]) << 32;
		case 4:  k1 ^= uint64_t(key[tail_start + 3]) << 24;
		case 3:  k1 ^= uint64_t(key[tail_start + 2]) << 16;
		case 2:  k1 ^= uint64_t(key[tail_start + 1]) << 8;
		case 1:  k1 ^= uint64_t(key[tail_start + 0]);
			k1 *= c1;
			k1 = murmur64::rotl64(k1, 31);
			k1 *= c2;
			h1 ^= k1;
		}

		h1 ^= len;
		h1 = murmur64::fmix64(h1);

		return h1;
	}

	inline constexpr uint64_t murmur_hash_64(const char* str, uint64_t seed = 0ull)
	{
		return murmur_hash_64(str, strlen(str), seed);
	}

	template<typename _Ty, typename _Traits = std::char_traits<_Ty>, typename _Allocator = std::allocator<_Ty>>
	inline uint64_t murmur_hash_64(const std::basic_string<_Ty, _Traits, _Allocator>& str, uint64_t seed = 0ull)
	{
		return murmur_hash_64(str.c_str(), str.size(), seed);
	}

	inline constexpr uint32_t operator "" _hash32(char const* s, size_t count) noexcept
	{
		return murmur_hash(s, count, 0);
	}

	inline constexpr uint64_t operator "" _hash64(char const* s, size_t count) noexcept
	{
		return murmur_hash_64(s, count, 0);
	}


	template<class _Ty>
	struct hash32
	{
		uint32_t operator()(const _Ty& data) const
		{
			return murmur_hash(reinterpret_cast<const char*>(&data), sizeof(_Ty), 0u);
		}
	};


	template<typename _Ty, typename _Traits, typename _Allocator>
	struct hash32<std::basic_string<_Ty, _Traits, _Allocator>>
	{
		uint32_t operator()(const std::basic_string<_Ty, _Traits, _Allocator>& data) const
		{
			return murmur_hash(reinterpret_cast<const char*>(data.c_str()), data.size() * sizeof(_Ty), 0u);
		}
	};

	template<class _Ty>
	struct hash64
	{
		uint64_t operator()(const _Ty& data) const
		{
			return murmur_hash_64(reinterpret_cast<const char*>(&data), sizeof(_Ty), 0ull);
		}
	};

	
	template<typename _Ty, typename _Traits, typename _Allocator>
	struct hash64<std::basic_string<_Ty, _Traits, _Allocator>>
	{
		uint64_t operator()(const std::basic_string<_Ty, _Traits, _Allocator>& data) const
		{
			return murmur_hash_64(reinterpret_cast<const char*>(data.c_str()), data.size() * sizeof(_Ty), 0ull);
		}
	};
}