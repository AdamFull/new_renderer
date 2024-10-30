#pragma once

namespace engine
{
	namespace render
	{
		class IDynamicBuffer
		{
		public:
			virtual ~IDynamicBuffer() = default;

			template<typename _Ty>
			void Emplace(uint64_t index, _Ty&& element)
			{

			}

			template<typename _Ty>
			_Ty& Get(uint64_t index)
			{
				return *reinterpret_cast<_Ty*>(GetBytes(CalculateOffset(index)));
			}

			template<typename _Ty>
			void InsertRaw(std::span<_Ty> insert_range, uint64_t offset = 0ull)
			{
				InsertBytes(insert_range.data(), insert_range.size() * sizeof(_Ty), offset);
			}

			template<typename _Ty>
			void Insert(std::span<_Ty> insert_range, uint64_t start_index = 0ull)
			{
				InsertBytes(insert_range.data(), insert_range.size() * sizeof(_Ty), CalculateOffset(start_index));
			}

			virtual void Flush() = 0;
		protected:
			virtual uint64_t CalculateOffset(uint64_t index) = 0;
			virtual void InsertBytes(const void* data, uint64_t size, uint64_t offset) = 0;
			virtual void* GetBytes(uint64_t offset) = 0;
		};
	}
}