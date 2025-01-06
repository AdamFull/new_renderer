#pragma once

#include <memory>
#include <cassert>

namespace edge
{
	namespace util
	{
		//template<typename _Ty> class Ref;

		template<typename _Ty>
		class Raw
		{
		public:
			Raw() = default;
			Raw(std::nullptr_t) noexcept : ptr{ nullptr } {}

			explicit Raw(_Ty* ptr) noexcept : ptr{ ptr } {}
			explicit Raw(const std::unique_ptr<_Ty>& unique_ptr) noexcept : ptr{ unique_ptr.get() } {}

			template<typename _RefTy>
			explicit Raw(const _RefTy& ref) noexcept : ptr{ ref.operator->() } {}

			template<typename _Derived>
			requires std::is_base_of_v<_Ty, _Derived>
			Raw(const Raw<_Derived>& other) noexcept : ptr{ other.ptr } {}

			explicit operator bool() const noexcept
			{
				return ptr != nullptr;
			}

			_Ty* operator->() const noexcept
			{
				assert(ptr && "Attempting to dereference a null pointer. Open your eyes!");
				return ptr;
			}
			
			template<typename _KTy>
			_KTy* As()
			{
				return reinterpret_cast<_KTy*>(ptr);
			}
		private:
			_Ty* ptr{ nullptr };
		};

		template<typename _Ty>
		class Ref
		{
			using pointer_t = _Ty*;
			using reference_t = std::unique_ptr<_Ty>&;
		public:
			explicit Ref(reference_t ref) noexcept : ref_ptr{ ref } {}

			explicit operator bool() const noexcept 
			{
				return ref_ptr;
			}

			pointer_t operator->() const noexcept
			{
				assert(ref_ptr.get() && "INVALID POINTER REFERENCE");
				return ref_ptr.get();
			}

			template<typename _KTy>
			[[nodiscard]] Raw<_KTy> As()
			{
				return Raw<_KTy>(reinterpret_cast<_KTy*>(ref_ptr.get()));
			}
		private:
			std::unique_ptr<_Ty>& ref_ptr;
		};
	}
}