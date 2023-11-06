#pragma once

#include <sol/forward.hpp>

namespace engine
{
	namespace render
	{
		void bindGLMath(sol::state* lua);
		void bindRenderInterface(sol::state* lua);
	}
}