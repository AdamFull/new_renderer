#pragma once

#include <unordered_map>

#include "BaseStructures.h"

namespace engine
{
	namespace render
	{
		enum class ERenderingType
		{
			eFullscreen,
			eGeometry,
			eCascadedShadowMap,
			eDirectionalShadowMap,
			eOmniShadowMap,
			eRenderScript,
			eClear,
			eImGui
		};

		enum class EComputeType
		{
			eDefault,
			eGeometry,
			eCubemap,
			eCulling
		};

		enum class ERenderingSizeType
		{
			eFromRenderTarget = 1,
			eFromBuffer = 2,
			eManual = 3
		};
	}
}