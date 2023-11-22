#pragma once

#include <vector>

namespace engine
{
	namespace render
	{
		enum EBufferTypeFlagBits
		{
			BUFFER_TYPE_STAGING_FLAG_BIT = 1 << 0,
			BUFFER_TYPE_VERTEX_FLAG_BIT = 1 << 1,
			BUFFER_TYPE_INDEX_FLAG_BIT = 1 << 2,
			BUFFER_TYPE_STORAGE_FLAG_BIT = 1 << 3,
			BUFFER_TYPE_UNIFORM_FLAG_BIT = 1 << 4
		};

		struct FBufferUploadInfo
		{
			size_t bufferId{ ~0ull };
			std::vector<uint8_t> data{};
			size_t count{ 1ull };
			size_t size{ 0ull };
			size_t offset{ 0ull };
		};

		struct FBufferCreateInfo
		{
			// EBufferTypeFlagBits
			uint32_t flags{ 0u };
			size_t count{ 1ull };
			size_t size{ 0ull };

			FBufferUploadInfo upload{};
		};

		struct FBufferView
		{
			size_t alignedSize{ 0ull };
			size_t elementCount{ 0ull };
		};

		class IBufferObject
		{
		public:
			virtual ~IBufferObject() = default;

			virtual void create(const std::string& name, const FBufferCreateInfo& create_info) = 0;

			virtual void map() = 0;
			virtual void unmap() = 0;
			virtual void write(void* data, size_t size = ~0ull, size_t offset = 0ull) = 0;
			virtual void flush(size_t size = ~0ull, size_t offset = 0ull) = 0;

			virtual char* at(size_t index) = 0;
		};
	}
}