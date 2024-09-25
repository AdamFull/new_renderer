#pragma once

#include <embed/shaders/includes/interop/base.h>
#include "BaseEnumerators.h"

namespace engine
{
	class Window;

	namespace render
	{
		enum class ERenderApi
		{
			eVulkan10,
			eVulkan11,
			eVulkan12,
			eVulkan13,
			eOpenGL,
			eDirectX12
		};

		struct FRenderInterfaceCreateInfo
		{
			ERenderApi render_api{};
			Window* window;
		};

		struct FRenderContextCreateInfo
		{
			std::string appName{ "dummy_app" };
			ERenderApi apiVersion{};

			Window* window;

			utl::vector<char> pipelineCache{};

			utl::vector<const char*> requiredDeviceExtensions{};
			utl::vector<const char*> optionalDeviceExtensions{};

			u32 validationFeatures{ 0u };
			utl::vector<const char*> validationLayers{};
		};

		template<class _Ty>
		struct FPoint2D
		{
			FPoint2D() = default;
			FPoint2D(_Ty val) : x(val), y(val) {}
			FPoint2D(_Ty _x, _Ty _y) : x(_x), y(_y) {}

			_Ty x{ 0 }, y{ 0 };
		};

		template<class _Ty>
		struct FPoint3D
		{
			FPoint3D() = default;
			FPoint3D(_Ty val) : x(val), y(val), z(val) {}
			FPoint3D(_Ty _x, _Ty _y, _Ty _z) : x(_x), y(_y), z(_z) {}

			_Ty x{ 0 }, y{ 0 }, z{ 0 };
		};

		template<class _Ty>
		struct FRect2D
		{
			FRect2D() = default;
			FRect2D(const FPoint2D<_Ty>& _offset, const FPoint2D<_Ty>& _size) : offset(_offset), size(_size) {}
			FRect2D(_Ty offset_x, _Ty offset_y, _Ty size_x, _Ty size_y) : offset(offset_x, offset_y), size(size_x, size_y) {}

			FPoint2D<_Ty> offset{};
			FPoint2D<_Ty> size{};
		};

		template<typename _Tag>
		struct FHandle
		{
			using ValueType = u64;

			// Default constructor
			FHandle() : value_(INVALID_HANDLE) {}

			// Parameterized constructor
			explicit FHandle(ValueType value) : value_(value) {}

			// Copy constructor
			FHandle(const FHandle& other) = default;

			// Move constructor
			FHandle(FHandle&& other) noexcept : value_(other.value_) { other.value_ = INVALID_HANDLE; }

			// Copy assignment operator
			FHandle& operator=(const FHandle& other) = default;

			// Move assignment operator
			FHandle& operator=(FHandle&& other) noexcept 
			{
				if (this != &other) 
				{
					value_ = other.value_;
					other.value_ = INVALID_HANDLE;
				}
				return *this;
			}

			// Conversion operator to uint64_t
			//operator ValueType() const { return value_; }
			ValueType Get() const { return value_; }

			// Boolean operator for validity check
			explicit operator bool() const { return value_ != INVALID_HANDLE; }

			explicit operator void* () const { return (void*)value_; }

			// Equality operator
			bool operator==(const FHandle & other) const { return value_ == other.value_; }

			// Inequality operator
			bool operator!=(const FHandle & other) const { return value_ != other.value_; }

			// Less than operator
			bool operator<(const FHandle & other) const { return value_ < other.value_; }

			// Greater than operator
			bool operator>(const FHandle & other) const { return value_ > other.value_; }

			// Less than or equal operator
			bool operator<=(const FHandle & other) const { return value_ <= other.value_; }

			// Greater than or equal operator
			bool operator>=(const FHandle & other) const { return value_ >= other.value_; }

			// Stream output operator
			friend std::ostream& operator<<(std::ostream & os, const FHandle & handle) 
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

		struct GraphTag {};
		struct RenderTag {};
		struct ApiTag {};

		using FGraphHandle = FHandle<GraphTag>;
		using FRHIHandle = FHandle<RenderTag>;
		using FAPIHandle = FHandle<ApiTag>;

		struct FGeometryDrawInfo
		{
			u64 lastIndex{ 0ull };
			u64 lastVertex{ 0ull };
			utl::vector<FGeometryDrawRange> ranges{};
		};

		struct FIndirectGeometryDrawInfo
		{
			FAPIHandle bufferId{ INVALID_HANDLE };
			u64 offset{ 0ull };

			FAPIHandle countBufferId{};
			u64 countOffset{ 0ull };

			u64 maxDrawCount{ 0ull };
			u64 stride{ 0ull };
		};

		struct FGeometryCPUBatch
		{
			FRHIHandle vboID{ INVALID_HANDLE };
			FIndirectGeometryDrawInfo command{};
		};

		struct FDispatchGroup
		{
			u32 x, y, z{};
		};

		struct FAPIObjectDestroyInfo
		{
			FAPIHandle id{ INVALID_HANDLE };
			EResourceFlagBits mask{};
		};
	}
}

namespace std
{
	template <typename Tag>
	struct hash<engine::render::FHandle<Tag>> 
	{
		std::size_t operator()(const engine::render::FHandle<Tag>& handle) const 
		{
			return std::hash<typename engine::render::FHandle<Tag>::ValueType>()(handle.Get());
			//return std::hash<typename engine::render::FHandle<Tag>::ValueType>()(static_cast<typename engine::render::FHandle<Tag>::ValueType>(handle));
		}
	};
}