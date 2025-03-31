#pragma once

#include "../rt_native_surface.h"

#include <Windows.h>

namespace edge::win64 {
	class WindowsNativeSurface : public NativeSurface {
	public:
		WindowsNativeSurface(HINSTANCE hinst, HWND hwnd) :
			hinst{ hinst },
			hwnd{ hwnd } {
		}

		~WindowsNativeSurface() = default;

		auto get_handle_instance() noexcept -> HINSTANCE& {
			return hinst;
		}

		auto get_handle() noexcept -> HWND& {
			return hwnd;
		}
	private:
		HINSTANCE hinst;
		HWND hwnd;
	};
}