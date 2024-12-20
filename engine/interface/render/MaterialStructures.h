#pragma once 

#include <string>

#include <embed/shaders/includes/shlib.h>

namespace engine
{
	namespace render
	{
		enum class EAlphaMode : uint32_t
		{
			eOpaque,
			eMask,
			eBlend
		};
	}
}