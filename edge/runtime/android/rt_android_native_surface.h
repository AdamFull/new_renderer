#pragma once

#include "../rt_native_surface.h"

namespace edge::android {
	class AndroidNativeSurface : public NativeSurface {
	public:
		AndroidNativeSurface(ANativeWindow*& window);
		~AndroidNativeSurface() = default;

		ANativeWindow*& get_handle();
	private:
		ANativeWindow*& handle;
	};
}