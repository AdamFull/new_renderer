#pragma once 

#include <string>

#include <embed/shaders/includes/interop/base.h>

namespace engine
{
	namespace render
	{
		enum class EAlphaMode : u32
		{
			eOpaque,
			eMask,
			eBlend
		};
	}
}