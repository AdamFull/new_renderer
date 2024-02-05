#pragma once

#include "BaseFunctions.h"

namespace engine
{
	namespace render
	{
		template<class _LightType, u64 _LightCount>
		struct FLightCommitData
		{
			u32 count{ 0u };
			stl::array<_LightType, _LightCount> lights;
		};

		template<class _ShadowType, class _BatchType, u64 _ShadowCount>
		struct FShadowCommitData
		{
			stl::array<_ShadowType, _ShadowCount> commits{};
			stl::array<_BatchType, _ShadowCount> batches{};
		};
	}
}