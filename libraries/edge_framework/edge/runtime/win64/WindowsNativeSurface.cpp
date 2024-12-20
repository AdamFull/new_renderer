#include "WindowsNativeSurface.h"

namespace edge
{
	namespace win32
	{
		WindowsNativeSurface::WindowsNativeSurface(HINSTANCE hinst, HWND hwnd) :
			hinst{ hinst },
			hwnd{ hwnd }
		{

		}

		HINSTANCE& WindowsNativeSurface::GetHandleInstance()
		{
			return hinst;
		}

		HWND& WindowsNativeSurface::GetHandle()
		{
			return hwnd;
		}
	}
}