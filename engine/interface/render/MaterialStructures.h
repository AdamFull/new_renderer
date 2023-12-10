#pragma once 

#include <string>

#include <embed/shaders/includes/interop/base.h>

namespace engine
{
	namespace render
	{
		enum EAlphaMode
		{
			MATERIAL_APLHA_MODE_OPAQUE,
			MATERIAL_APLHA_MODE_MASK,
			MATERIAL_APLHA_MODE_BLEND
		};

		struct FMaterialCreateInfo
		{
			std::string shaderName{};
			FMaterial parameters{};
			size_t usageCount{ 0ull };
		};
	}
}