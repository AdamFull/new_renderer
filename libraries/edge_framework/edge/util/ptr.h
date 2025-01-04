#pragma once

#include <memory>
#include <cassert>

namespace edge
{
	namespace util
	{
		template<typename _Ty>
		class Ref
		{
			using pointer_t = _Ty*;
			using reference_t = std::unique_ptr<_Ty>&;
		public:
			explicit Ref(reference_t ref) noexcept :
				ref_ptr{ ref }
			{
			}

			explicit operator bool() const noexcept 
			{
				return ref_ptr;
			}

			pointer_t operator->() const noexcept
			{
				assert(ref_ptr.get() && "INVALID POINTER REFERENCE");
				return ref_ptr.get();
			}
		private:
			std::unique_ptr<_Ty>& ref_ptr;
		};
	}
}