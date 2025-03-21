#pragma once

#include "../rt_native_surface.h"

struct GLFWwindow;

namespace edge {
	NativeSurface* make_glfw_native_surface(GLFWwindow* handle);
}