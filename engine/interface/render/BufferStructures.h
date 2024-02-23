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
			virtual void resize(const stl::string& name, const FBufferCreateInfo& create_info) = 0;

			virtual void map() = 0;
			virtual void unmap() = 0;
			virtual void write(const void* data, u64 size = ~0ull, u64 offset = 0ull) = 0;
			virtual void flush(u64 size = ~0ull, u64 offset = 0ull) = 0;
			virtual void clear() = 0;

			virtual u64 size() = 0;
			virtual u64 count() = 0;

			virtual u64 handle() = 0;
			virtual uint64_t address() = 0;

			virtual char* at(u64 index) = 0;
		};

		class IGPUVector
		{
		public:
			virtual ~IGPUVector() = default;

			template<class _Ty>
			void create(const std::string& name, EBufferTypeFlagBits flag = EBufferTypeFlagBits::eStorage, u64 capacity = 64ull)
			{
				//m_elementType = typeid(_Ty);
				alloc(name, flag, sizeof(_Ty), capacity);
			}

			template<class _Ty>
			void copy(const stl::vector<_Ty>& data)
			{
				if (m_size + data.size() > capacity())
				{
					u64 new_capacity = (m_size + data.size()) * 2ull;
					realloc(new_capacity);
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
					realloc(new_capacity);
				}

				auto* memory_block = at(m_size);
				std::memcpy(memory_block, &new_element, sizeof(_Ty));
				++m_size;

				return static_cast<_Ty*>(memory_block);
			}

			void clear()
			{
				m_size = 0ull;
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

			virtual void resize(u64 new_size) = 0;

			u64 size() { return m_size; }

			virtual u64 capacity() = 0;

			virtual const stl::unique_ptr<IBufferObject>& buffer() = 0;

		protected:
			virtual void alloc(const std::string& name, EBufferTypeFlagBits flag, u64 stride, u64 capacity) = 0;
			virtual void realloc(u64 capacity) = 0;

			virtual void* at(u64 index) = 0;
			virtual const void* at(u64 index) const = 0;
		protected:
			u64 m_size{};
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