#pragma once

#include <cstdint>
#include <ostream>

namespace engine
{
	namespace render
	{
		template<typename _Tag>
		struct ResourceHandle
		{
			using ValueType = uint32_t;

			// Default constructor
			ResourceHandle() : value_(~0u) {}

			// Parameterized constructor
			explicit ResourceHandle(ValueType value) : 
				value_(value)
			{}

			// Copy constructor
			ResourceHandle(const ResourceHandle& other) = default;

			// Move constructor
			ResourceHandle(ResourceHandle&& other) noexcept :
				value_{ std::exchange(other.value_, ~0u) },
				revision_{ std::exchange(other.value_, {} ) }
			{
			}

			// Copy assignment operator
			ResourceHandle& operator=(const ResourceHandle& other) = default;

			// Move assignment operator
			ResourceHandle& operator=(ResourceHandle&& other) noexcept
			{
				if (this != &other)
				{
					value_ = other.value_;
					other.value_ = ~0u;
				}
				return *this;
			}

			// Conversion operator to uint64_t
			operator ValueType() const 
			{ 
				return value_; 
			}

			ValueType Get() const 
			{ 
				return value_; 
			}

			// Boolean operator for validity check
			explicit operator bool() const 
			{ 
				return value_ != ~0u;
			}

			explicit operator void* () const 
			{ 
				return (void*)value_; 
			}

			// Equality operator
			bool operator==(const ResourceHandle& other) const 
			{ 
				return value_ == other.value_ && revision_ == other.revision_; 
			}

			// Inequality operator
			bool operator!=(const ResourceHandle& other) const
			{
				return value_ != other.value_ && revision_ != other.revision_;
			}

			// Less than operator
			bool operator<(const ResourceHandle& other) const 
			{ 
				return value_ < other.value_; 
			}

			// Greater than operator
			bool operator>(const ResourceHandle& other) const 
			{ 
				return value_ > other.value_; 
			}

			// Less than or equal operator
			bool operator<=(const ResourceHandle& other) const 
			{ 
				return value_ <= other.value_; 
			}

			// Greater than or equal operator
			bool operator>=(const ResourceHandle& other) const 
			{ 
				return value_ >= other.value_; 
			}

			// Stream output operator
			friend std::ostream& operator<<(std::ostream& os, const ResourceHandle& handle)
			{
				if (handle)
					os << handle.value_;
				else
					os << "INVALID_HANDLE";

				return os;
			}

		private:
			ValueType value_;
			ValueType revision_;
		};

		struct RendererHandle {};

		using Handle = ResourceHandle<RendererHandle>;
	}
}