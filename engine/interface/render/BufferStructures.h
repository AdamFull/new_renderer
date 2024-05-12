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
			eGpuOnly = eVertex | eIndex,
			eAlignmentRequired = eVertex | eIndex | eUniform,
			eHasDeviceAddress = eStorage | eVertex | eIndex | eIndirect,
			eCpuToGpu = eStorage | eUniform | eIndirect,

			eStagingBuffer = eTransfer,
			eStorageBuffer = eTransfer | eStorage,
			eVertexBuffer = eTransfer | eVertex,
			eIndexBuffer = eTransfer | eIndex,
			eUniformBuffer = eTransfer | eUniform,
			eIndirectBuffer = eTransfer | eStorage | eIndirect
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

		class IGPUVector2
		{
		public:
			struct upload_range
			{
				u64 offset{ 0ull };
				u64 size{ 0ull };
			};

			virtual ~IGPUVector2() = default;

			template<class _Ty>
			void create(const std::string& name, EBufferUsageFlags flag = EBufferUsageFlagBits::eStorageBuffer, u64 capacity = 64ull)
			{
				// Init intermediate data buffer
				m_vData.resize(sizeof(_Ty) * capacity);
				// Allocate buffer
				allocate(name, flag, sizeof(_Ty), capacity);
			}

			template<class _Ty, size_t _Size>
			void copy(const std::array<_Ty, _Size>& data)
			{
				assert(m_elemSize == sizeof(_Ty) && "Invalid element size.");

				u64 size = to_bytes(_Size);
				try_grow(size);
				do_copy(&data[0], size);
			}

			template<class _Ty>
			void copy(const utl::vector<_Ty>& data)
			{
				assert(m_elemSize == sizeof(_Ty) && "Invalid element size.");

				if (data.empty())
					return;

				u64 size = to_bytes(data.size());
				try_grow(size);
				do_copy(&data[0], size);
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

			template<class _Ty>
			void set(u64 index, _Ty&& data)
			{
				assert(m_elemSize == sizeof(_Ty) && "Invalid element size.");
				do_insert(&data, m_elemSize, to_bytes(index));
			}

			template<class _Ty>
			void erase(u64 count)
			{
				std::memset(&m_vData[0], 0, sizeof(_Ty) * count);
			}

			u64 count()
			{
				return m_vData.size() / m_elemSize;
			}

			//template<class _Ty>
			//_Ty* at(u64 index)
			//{
			//	m_vUploadRanges.emplace_back(to_bytes(index), m_elemSize);
			//	return (_Ty*)(&m_vData[to_bytes(index)]);
			//}

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
			void try_grow(u64 size)
			{
				if (m_cursor + size > m_vData.size())
				{
					u64 new_size = (m_cursor + size) * 1.5f;
					m_vData.resize(new_size);
				}
			}

			void do_insert(const void* data, u64 size, u64 offset)
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
			void do_copy(const void* data, u64 size)
			{
				do_insert(data, size, m_cursor);
				m_cursor += size;
			}

			// Allocates buffer
			virtual void allocate(const std::string& name, EBufferUsageFlags flag, u64 stride, u64 capacity) = 0;

			utl::vector<upload_range> m_vUploadRanges{};
			utl::vector<u8> m_vData{};
			u64 m_cursor{ 0ull };	// Something like cursor in bytes
			u64 m_elemSize{ 0ull };
		};

		class IGPUVector
		{
		public:
			virtual ~IGPUVector() = default;

			template<class _Ty>
			void create(const std::string& name, EBufferUsageFlags flag = EBufferUsageFlagBits::eStorageBuffer, u64 capacity = 64ull)
			{
				//m_elementType = typeid(_Ty);
				allocate(name, flag, sizeof(_Ty), capacity);
			}

			template<class _Ty, size_t _Size>
			void copy(const std::array<_Ty, _Size>& data)
			{
				if (m_size + _Size > capacity())
				{
					u64 new_capacity = (m_size + _Size) * 2ull;
					reallocate(new_capacity);
				}

				auto* memory_block = at(m_size);
				std::memcpy(memory_block, data.data(), sizeof(_Ty) * _Size);
				m_size += _Size;
			}

			template<class _Ty>
			void copy(const utl::vector<_Ty>& data)
			{
				if (m_size + data.size() > capacity())
				{
					u64 new_capacity = (m_size + data.size()) * 2ull;
					reallocate(new_capacity);
				}

				auto* memory_block = at(m_size);
				std::memcpy(memory_block, data.data(), sizeof(_Ty) * data.size());
				m_size += data.size();
			}

			template<class _Ty>
			_Ty* emplace_back(const _Ty& new_element = _Ty())
			{
				//assert(std::type_index(typeid(_Ty)) == m_elementType);

				if (m_size + 1ull > capacity())
				{
					u64 new_capacity = m_size == 0ull ? 1ull : m_size * 2ull;
					reallocate(new_capacity);
				}

				auto* memory_block = static_cast<_Ty*>(at(m_size));
				*memory_block = new_element;
				//std::memcpy(memory_block, &new_element, sizeof(_Ty));
				++m_size;

				return memory_block;
			}

			void clear()
			{
				m_size = 0ull;
			}

			template<class _Ty>
			void erase(u64 count)
			{
				std::memset(at(0), 0, sizeof(_Ty) * count);
			}

			template<class _Ty>
			_Ty& get(u64 index)
			{
				//assert(std::type_index(typeid(_Ty)) == m_elementType);
				return *static_cast<_Ty*>(at(index));
			}

			template<class _Ty>
			const _Ty& get(u64 index) const
			{
				//assert(std::type_index(typeid(_Ty)) == m_elementType);
				return *static_cast<const _Ty*>(at(index));
			}

			template<class _Ty>
			_Ty& front()
			{
				assert(m_size > 0ull);
				return get<_Ty>(0ull);
			}

			template<class _Ty>
			const _Ty& front() const
			{
				assert(m_size > 0ull);
				return get<_Ty>(0ull);
			}

			template<class _Ty>
			_Ty& back()
			{
				assert(m_size > 0ull);
				return get<_Ty>(m_size - 1ull);
			}

			template<class _Ty>
			const _Ty& back() const
			{
				assert(m_size > 0ull);
				return get<_Ty>(m_size - 1ull);
			}

			//virtual void map() = 0;
			//virtual void unmap() = 0;

			virtual void resize(u64 new_size) = 0;

			u64 size() { return m_size; }

			virtual u64 capacity() = 0;

			virtual const std::unique_ptr<IBufferObject>& buffer() = 0;

		protected:
			virtual void allocate(const std::string& name, EBufferUsageFlags flag, u64 stride, u64 capacity) = 0;
			virtual void reallocate(u64 capacity) = 0;

			virtual void* at(u64 index) = 0;
			virtual const void* at(u64 index) const = 0;
		protected:
			u64 m_size{};
		};
	}
}