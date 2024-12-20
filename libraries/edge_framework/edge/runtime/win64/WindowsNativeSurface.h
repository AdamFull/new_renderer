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
			WindowsNativeSurface(HINSTANCE hinst, HWND hwnd);
			~WindowsNativeSurface() = default;

			HINSTANCE& GetHandleInstance();
			HWND& GetHandle();
		private:
			HINSTANCE hinst;
			HWND hwnd;
		};
	}
}