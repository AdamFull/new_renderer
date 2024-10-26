#pragma once

#include <engine/rendering/Common.h>

namespace engine
{
	namespace render
	{
		struct Handle
		{
			using ValueType = uint32_t;

			// Default constructor
			Handle() : value_(~0u) {}

			// Parameterized constructor
			explicit Handle(ValueType value) :
				value_(value)
			{}

			// Copy constructor
			Handle(const Handle& other) = default;

			// Move constructor
			Handle(Handle&& other) noexcept :
				value_{ std::exchange(other.value_, ~0u) }
			{
			}

			// Copy assignment operator
			Handle& operator=(const Handle& other) = default;

			// Move assignment operator
			Handle& operator=(Handle&& other) noexcept
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
			bool operator==(const Handle& other) const
			{
				return value_ == other.value_;
			}

			// Inequality operator
			bool operator!=(const Handle& other) const
			{
				return value_ != other.value_;
			}

			// Less than operator
			bool operator<(const Handle& other) const
			{
				return value_ < other.value_;
			}

			// Greater than operator
			bool operator>(const Handle& other) const
			{
				return value_ > other.value_;
			}

			// Less than or equal operator
			bool operator<=(const Handle& other) const
			{
				return value_ <= other.value_;
			}

			// Greater than or equal operator
			bool operator>=(const Handle& other) const
			{
				return value_ >= other.value_;
			}

			// Stream output operator
			friend std::ostream& operator<<(std::ostream& os, const Handle& handle)
			{
				if (handle)
					os << handle.value_;
				else
					os << "INVALID_HANDLE";

				return os;
			}

		private:
			ValueType value_;
		};

		template<class _Ty>
		class ResourceContainer final
		{
		public:
			Handle Add(_Ty&& object)
			{
				static std::atomic<uint32_t> uid{ 0u };

				if (!free_ids.empty())
				{
					uint32_t current_id = free_ids.front();
					free_ids.pop();

					objects[current_id] = std::move(object);

					return Handle{ current_id };
				}

				uint32_t current_id = uid.load(std::memory_order_relaxed);
				uid.fetch_add(1ull, std::memory_order_relaxed);

				objects.emplace_back(std::move(object));

				return Handle{ current_id };
			}

			void Remove(Handle id)
			{
				free_ids.push(id);
				objects[id] = nullptr;
			}

			const _Ty& Get(Handle id) const
			{
				return objects[static_cast<Handle::ValueType>(id)];
			}

			_Ty& Get(Handle id)
			{
				return objects[static_cast<Handle::ValueType>(id)];
			}

			std::vector<_Ty>& data()
			{
				return objects;
			}
		private:
			std::queue<Handle::ValueType> free_ids{};
			std::vector<_Ty> objects{};
		};
	}
}