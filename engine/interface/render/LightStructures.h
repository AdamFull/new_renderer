#pragma once

#include "BaseFunctions.h"

#include <embed/shaders/includes/interop/base.h>

namespace engine
{
	namespace render
	{
		template<class _LightType, size_t _LightCount>
		struct FLightCommitData
		{
			uint32_t count{ 0u };
			std::array<_LightType, _LightCount> lights;
		};

		template<class _ShadowType, class _BatchType, size_t _ShadowCount>
		struct FShadowCommitData
		{
			std::array<_ShadowType, _ShadowCount> commits{};
			std::array<_BatchType, _ShadowCount> batches{};
		};

		// TODO: make it dynamic
		using directional_lights_t = FLightCommitData<FDirectionalLightCommit, max_directional_light_count>;
		using cascaded_shadows_t = FShadowCommitData<FCascadedShadowCommit, std::array<std::vector<FGeometryBatch>, cascade_shadow_map_cascade_count>, max_directional_light_shadow_count>;

		struct FDirectionalLightsData
		{
			directional_lights_t lights{};
			cascaded_shadows_t shadows{};
		};

		using point_lights_t = FLightCommitData<FPointLightCommit, max_point_light_count>;
		using omni_shadows_t = FShadowCommitData<FOmniShadowCommit, std::array<std::vector<FGeometryBatch>, 6ull>, max_point_light_shadow_count>;

		struct FPointLightsData
		{
			point_lights_t lights{};
			omni_shadows_t shadows{};
		};

		using spot_lights_t = FLightCommitData<FSpotLightCommit, max_spot_light_count>;
		using spot_shadows_t = FShadowCommitData<FSpotShadowCommit, std::vector<FGeometryBatch>, max_spot_light_shadow_count>;

		struct FSpotLightsData
		{
			spot_lights_t lights{};
			spot_shadows_t shadows{};
		};
	}
}