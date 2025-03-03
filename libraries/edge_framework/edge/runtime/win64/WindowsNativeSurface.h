#pragma once

#include "../NativeSurface.h"

#include <Windows.h>

namespace edge
{
	namespace win32
	{
		class WindowsNativeSurface : public NativeSurface
		{
		public:
			WindowsNativeSurface(HINSTANCE hinst, HWND hwnd) :
				hinst{ hinst },
				hwnd{ hwnd }
			{
			}

			~WindowsNativeSurface() = default;

			auto GetHandleInstance() noexcept -> HINSTANCE&
			{
				return hinst;
			}

			auto GetHandle() noexcept -> HWND&
			{
				return hwnd;
			}
		private:
			HINSTANCE hinst;
			HWND hwnd;
		};
	}
}