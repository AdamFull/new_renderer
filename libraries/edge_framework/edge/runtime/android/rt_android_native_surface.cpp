#include "rt_android_native_surface.h"

namespace edge::android
{
	AndroidNativeSurface::AndroidNativeSurface(ANativeWindow*& window) :
		handle{ window } {
	}

	ANativeWindow*& AndroidNativeSurface::get_handle() {
		return handle;
	}
}