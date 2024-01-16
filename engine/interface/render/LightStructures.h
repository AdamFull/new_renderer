#pragma once

#include "BaseFunctions.h"

#include <embed/shaders/includes/interop/base.h>

namespace engine
{
	namespace render
	{
		template<class _LightType, u64 _LightCount>
		struct FLightCommitData
		{
			u32 count{ 0u };
			std::array<_LightType, _LightCount> lights;
		};

		template<class _ShadowType, class _BatchType, u64 _ShadowCount>
		struct FShadowCommitData
		{
			std::array<_ShadowType, _ShadowCount> commits{};
			std::array<_BatchType, _ShadowCount> batches{};
		};
	}
}