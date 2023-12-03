#pragma once

#include "BaseFunctions.h"

namespace engine
{
	namespace render
	{
		// Directional light
		constexpr const uint32_t max_directional_light_count{ 3u };
		constexpr const uint32_t max_directional_light_shadow_count{ 1u };
		constexpr const uint32_t cascade_shadow_map_resolution{ 2048u };
		constexpr const uint32_t cascade_shadow_map_cascade_count{ 8u };

		// Spot light
		constexpr const uint32_t max_spot_light_count{ 64u };
		constexpr const uint32_t max_spot_light_shadow_count{ 1u };
		constexpr const uint32_t spot_light_shadow_map_resolution{ 1024u };

		// Point light
		constexpr const uint32_t max_point_light_count{ 64u };
		constexpr const uint32_t max_point_light_shadow_count{ 1u };
		constexpr const uint32_t point_light_shadow_map_resolution{ 512u };

		struct FCascadedShadowCommit
		{
			std::array<float, cascade_shadow_map_cascade_count> splitDepths;
			std::array<glm::mat4, cascade_shadow_map_cascade_count> cascadeViewProj;
			alignas(4) float farClip;
		};

		struct FDirectionalLightCommit
		{
			alignas(16) glm::vec3 direction;
			alignas(16) glm::vec3 color;
			alignas(4) float intencity;
			alignas(4) int shadowIndex;
			alignas(4) int castShadows;
		};

		struct FOmniShadowCommit
		{
			std::array<glm::mat4, 6u> viewProj;
			alignas(4) float farPlane;
			alignas(4) int index;
		};

		struct FPointLightCommit
		{
			alignas(16) glm::vec3 position;
			alignas(16) glm::vec3 color;
			alignas(4) float intencity;
			alignas(4) float radius;
			alignas(4) int shadowIndex;
			alignas(4) int castShadows;
		};

		struct FSpotShadowCommit
		{
			glm::mat4 viewProj;
			alignas(4) int index;
		};

		struct FSpotLightCommit
		{
			alignas(16) glm::vec3 position;
			alignas(16) glm::vec3 direction;
			alignas(16) glm::vec3 color;
			alignas(4) float intencity;
			alignas(4) float lightAngleScale;
			alignas(4) float lightAngleOffset;
			alignas(4) int toTarget;
			alignas(4) int shadowIndex;
			alignas(4) int castShadows;
		};

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