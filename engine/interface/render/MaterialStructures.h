#pragma once 

#include <string>

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