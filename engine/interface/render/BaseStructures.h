#pragma once

#if defined(PLATFORM_WIN32)
#include <Windows.h>
#endif

#include <embed/shaders/includes/interop/base.h>
#include "BaseEnumerators.h"

namespace engine
{
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

#if defined( PLATFORM_WIN32 )
			HINSTANCE hinstance;
			HWND hwnd;
#endif

			i32 initialWidth{};
			i32 initialHeight{};
		};

		struct FRenderContextCreateInfo
		{
			std::string appName{ "dummy_app" };
			ERenderApi apiVersion{};

#if defined( VK_USE_PLATFORM_WIN32_KHR )
			HINSTANCE                       hinstance;
			HWND                            hwnd;
#endif
			utl::vector<char> pipelineCache{};

			utl::vector<std::string> requiredDeviceExtensions{};
			utl::vector<std::string> optionalDeviceExtensions{};

			u32 initialWidth{ 0u };
			u32 initialHeight{ 0u };

			bool validationState{ false };
			u32 validationFeatures{ 0u };
			utl::vector<std::string> validationLayers{};
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

		struct FGeometryDrawInfo
		{
			u64 lastIndex{ 0ull };
			u64 lastVertex{ 0ull };
			utl::vector<FGeometryDrawRange> ranges{};
		};

		struct FIndirectGeometryDrawInfo
		{
			u64 bufferId{ INVALID_HANDLE };
			u64 offset{ 0ull };

			u64 countBufferId{ INVALID_HANDLE };
			u64 countOffset{ 0ull };

			u64 maxDrawCount{ 0ull };
			u64 stride{ 0ull };
		};

		struct FGeometryCPUBatch
		{
			u64 vboID{ INVALID_HANDLE };
			FIndirectGeometryDrawInfo command{};
		};

		struct FDispatchGroup
		{
			u32 x, y, z{};
		};

		struct FAPIObjectDestroyInfo
		{
			u64 id{ INVALID_HANDLE };
			EResourceFlagBits mask{};
		};
	}
}