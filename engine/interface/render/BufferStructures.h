#pragma once

#include <standart_library.h>
#include <container/flags.h>
#include <utils/hash.h>

namespace engine
{
	namespace render
	{
		enum class EBufferUsageFlagBits
		{
			eTransfer = 1 << 0,
			eStorage = 1 << 1,
			eVertex = 1 << 2,
			eIndex = 1 << 3,
			eUniform = 1 << 4,
			eIndirect = 1 << 5,
			eGpuOnly = 1 << 6,
			eCpuToGpu = 1 << 7,

			eHasDeviceAddress = eVertex | eIndex | eStorage | eIndirect,

			eStagingBuffer = eTransfer,
			eStorageBufferCPU = eTransfer | eStorage | eCpuToGpu,
			eStorageBuffer = eTransfer | eStorage | eGpuOnly,
			eVertexBuffer = eTransfer | eVertex | eGpuOnly,
			eIndexBuffer = eTransfer | eIndex | eGpuOnly,
			eUniformBuffer = eTransfer | eUniform | eCpuToGpu,
			eIndirectBuffer = eTransfer | eStorage | eIndirect | eGpuOnly
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(EBufferUsageFlags, EBufferUsageFlagBits)

		struct FBufferUploadRegion
		{
			u64 count{ 1ull };
			u64 size{ 0ull };

			u64 srcOffset{ 0ull };
			u64 dstOffset{ 0ull };
		};

		struct FBufferUploadInfo
		{
			u64 bufferId{ ~0ull };
			utl::vector<u8> data{};
			utl::vector<FBufferUploadRegion> regions{};
		};

		struct FBufferCreateInfo
		{
			// EBufferTypeFlagBits
			EBufferUsageFlags flags{};
			u64 count{ 1ull };
			u64 size{ 0ull };
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

			virtual void create(const std::string& name, const FBufferCreateInfo& create_info) = 0;
			virtual void resize(const std::string& name, const FBufferCreateInfo& create_info) = 0;
			virtual void upload(utl::vector<u8>&& data, const utl::vector<FBufferUploadRegion>& regions) = 0;

			virtual void map() = 0;
			virtual void unmap() = 0;
			virtual void write(const void* data, u64 size = ~0ull, u64 offset = 0ull) = 0;
			virtual void flush(u64 size = ~0ull, u64 offset = 0ull) = 0;
			virtual void clear() = 0;

			virtual u64 size() = 0;
			virtual u64 instanceSize() = 0;
			virtual u64 count() = 0;

			virtual u64 handle() = 0;
			virtual uint64_t address() = 0;

			virtual void* data() = 0;
		};

		class IGPUVector
		{
		public:
			struct upload_range
			{
				u64 offset{ 0ull };
				u64 size{ 0ull };
			};

			virtual ~IGPUVector() = default;

			inline void create_raw(const std::string& name, u64 instance_size, u64 capacity = 64ull, EBufferUsageFlags flag = EBufferUsageFlagBits::eStorageBuffer)
			{
				// Init intermediate data buffer
				m_vData.resize(instance_size * capacity);
				// Allocate buffer
				allocate(name, flag, instance_size, capacity);
			}

			template<class _Ty>
			void create(const std::string& name, u64 capacity = 64ull, EBufferUsageFlags flag = EBufferUsageFlagBits::eStorageBuffer)
			{
				create_raw(name, sizeof(_Ty), capacity, flag);
			}

			inline void copy_bytes(const void* data, u64 byte_count)
			{
				try_grow(byte_count);
				do_copy(data, byte_count);
			}

			inline void copy_raw(const void* data, u64 count)
			{
				u64 size = to_bytes(count);
				copy_bytes(data, size);
			}

			template<class _Ty, size_t _Size>
			void copy(const std::array<_Ty, _Size>& data)
			{
				assert(m_elemSize == sizeof(_Ty) && "Invalid element size.");
				copy_raw(&data[0], _Size);
			}

			template<class _Ty>
			void copy(const utl::vector<_Ty>& data)
			{
				assert(m_elemSize == sizeof(_Ty) && "Invalid element size.");

				if (data.empty())
					return;

				copy_raw(&data[0], data.size());
			}

			template<class _Ty>
			_Ty* emplace_back(const _Ty& new_element = _Ty())
			{
				assert(m_elemSize == sizeof(_Ty) && "Invalid element size.");

				u64 size = sizeof(_Ty);
				try_grow(size);
				do_copy(&new_element, size);

				return (_Ty*)(&m_vData[m_cursor - size]);
			}

			inline void set_raw(u64 index, const void* data)
			{
				do_insert(data, m_elemSize, to_bytes(index));
			}

			template<class _Ty>
			void set(u64 index, _Ty&& data)
			{
				assert(m_elemSize == sizeof(_Ty) && "Invalid element size.");
				set_raw(index, &data);
			}

			u64 pos() const
			{
				return m_cursor / m_elemSize;
			}

			u64 count() const
			{
				return m_lastCursor / m_elemSize;
			}

			u64 capacity() const
			{
				return m_vData.size() / m_elemSize;
			}

			u64 stride() const
			{
				return m_elemSize;
			}

			template<class _Ty>
			const _Ty* at(u64 index) const
			{
				return (const _Ty*)(&m_vData[to_bytes(index)]);
			}

			virtual void upload() = 0;
			virtual void resize(u64 new_size) = 0;
			virtual const std::unique_ptr<IBufferObject>& buffer() = 0;
		protected:
			inline u64 to_bytes(u64 size) const { return size * m_elemSize; }

			// Try to grow temporary array
			inline void try_grow(u64 size)
			{
				if (m_cursor + size > m_vData.size())
				{
					u64 new_size = (m_cursor + size) * 2.f;
					m_vData.resize(new_size);
				}
			}

			inline void do_insert(const void* data, u64 size, u64 offset)
			{
				auto new_hash = utl::murmur_hash_64(data, size, 0);
				auto old_hash = utl::murmur_hash_64(&m_vData[offset], size, 0);

				if (new_hash != old_hash)
				{
					std::memcpy(&m_vData[offset], data, size);
					m_vUploadRanges.emplace_back(upload_range{ offset, size });
				}
			}

			// Copy data only when it changed ans det dirty flag
			inline void do_copy(const void* data, u64 size)
			{
				do_insert(data, size, m_cursor);
				m_cursor += size;
				m_lastCursor = m_cursor;
			}

			// Allocates buffer
			virtual void allocate(const std::string& name, EBufferUsageFlags flag, u64 stride, u64 capacity) = 0;

			utl::vector<upload_range> m_vUploadRanges{};
			utl::vector<u8> m_vData{};
			u64 m_cursor{ 0ull };	// Something like cursor in bytes
			u64 m_lastCursor{ 0ull };
			u64 m_elemSize{ 0ull };
		};
	}
}