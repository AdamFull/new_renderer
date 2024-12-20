#pragma once

#include "../NativeSurface.h"

struct GLFWwindow;

namespace edge
{
	NativeSurface* MakeGlfwNativeSurface(GLFWwindow* handle);
}