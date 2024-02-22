#pragma once

#include <standart_library.h>
#include <container/flags.h>

namespace engine
{
	namespace render
	{
		enum class EBufferTypeFlagBits
		{
			eStaging = 1 << 0,
			eVertex = 1 << 1,
			eIndex = 1 << 2,
			eStorage = 1 << 3,
			eUniform = 1 << 4,
			eIndirect = 1 << 5,
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(EBufferTypeFlags, EBufferTypeFlagBits)

		struct FBufferUploadInfo
		{
			u64 bufferId{ ~0ull };
			stl::vector<uint8_t> data{};
			u64 count{ 1ull };
			u64 size{ 0ull };
			u64 offset{ 0ull };
		};

		struct FBufferCreateInfo
		{
			// EBufferTypeFlagBits
			EBufferTypeFlagBits flags{};
			u64 count{ 1ull };
			u64 size{ 0ull };

			FBufferUploadInfo upload{};
		};

		struct FBufferView
		{
			u64 alignedSize{ 0ull };
			u64 elementCount{ 0ull };
			uint64_t address{ 0ull };
		};

		class IBufferObject
		{
		public:
			virtual ~IBufferObject() = default;

			virtual void create(const stl::string& name, const FBufferCreateInfo& create_info) = 0;
			virtual void resize(u64 size) = 0;

			virtual void map() = 0;
			virtual void unmap() = 0;
			virtual void write(const void* data, u64 size = ~0ull, u64 offset = 0ull) = 0;
			virtual void flush(u64 size = ~0ull, u64 offset = 0ull) = 0;
			virtual void clear() = 0;

			virtual u64 handle() = 0;
			virtual uint64_t address() = 0;

			virtual char* at(u64 index) = 0;
		};

		class IGPUArrayHandle
		{
		public:
			virtual ~IGPUArrayHandle() = default;

			virtual void create(const stl::string& name, u64 stride, u64 count) = 0;
			virtual void flush(u64 count = ~0ull, u64 offset = 0ull) = 0;
			virtual void sync(u64 count) = 0;

			virtual uint8_t* at(u64 index) = 0;
			virtual const uint8_t* at(u64 index) const = 0;

			virtual void insert(const void* data, u64 index) = 0;

			virtual uint8_t* emplace_back(const void* data) = 0;

			virtual uint8_t* back() = 0;
			virtual const uint8_t* back() const = 0;

			virtual void clear() = 0;

			virtual const stl::vector<uint8_t>& getData() const = 0;
			virtual const stl::unique_ptr<IBufferObject>& getBufferObject() const = 0;

			virtual u64 size() const = 0;
		};
	}
}