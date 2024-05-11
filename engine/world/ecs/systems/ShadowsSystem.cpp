#include "ShadowsSystem.h"

#include <engine/world/SceneGraph.h>

#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>
#include <engine/world/ecs/components/DirectionalLightComponent.h>
#include <engine/world/ecs/components/PointLightComponent.h>
#include <engine/world/ecs/components/SpotLightComponent.h>
#include <engine/world/ecs/components/MeshComponent.h>

#include <engine/profiler/Profiler.h>

#include <glm/gtc/matrix_transform.hpp>

using namespace engine::render;
using namespace engine::world::ecs;

constexpr const f32 cascade_split_lambda = 0.85f;

CShadowsSystem::CShadowsSystem(CSceneGraph* scenegraph, IRenderInterface* render_interface)
{
	m_pSceneGraph = scenegraph;
	m_pRenderInterface = render_interface;
}

void CShadowsSystem::__create()
{
	auto& render_world = m_pRenderInterface->getRenderWorld();

	{
		auto& culler_group = m_shadowCullers[ERenderGeometryCullerType::eCascadedShadow];
		for (u64 caster_id = 0ull; caster_id < max_directional_light_shadow_count; ++caster_id)
		{
			auto& culler_list = culler_group.emplace_back();
			for (u64 frustum_id = 0ull; frustum_id < cascade_shadow_map_cascade_count; ++frustum_id)
				culler_list.emplace_back(render_world->registerCamera(ERenderGeometryCullerType::eCascadedShadow));
		}
	}

	{
		auto& culler_group = m_shadowCullers[ERenderGeometryCullerType::eOmniShadow];
		for (u64 caster_id = 0ull; caster_id < max_point_light_shadow_count; ++caster_id)
		{
			auto& culler_list = culler_group.emplace_back();
			for (u64 frustum_id = 0ull; frustum_id < 6u; ++frustum_id)
				culler_list.emplace_back(render_world->registerCamera(ERenderGeometryCullerType::eOmniShadow));
		}
	}

	{
		auto& culler_group = m_shadowCullers[ERenderGeometryCullerType::eSpotShadow];
		for (u64 caster_id = 0ull; caster_id < max_spot_light_shadow_count; ++caster_id)
		{
			auto& culler_list = culler_group.emplace_back();
			culler_list.emplace_back(render_world->registerCamera(ERenderGeometryCullerType::eSpotShadow));
		}
	}
}

void CShadowsSystem::__update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = m_pSceneGraph->getRegistry();
	auto& render_packet = m_pRenderInterface->getRenderPacket();
	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& render_graph = m_pRenderInterface->getRenderGraph();
	auto& world_stats = render_world->getStats();

	auto& camera = render_world->getDefaultCamera();

	// Calculate cascades shadow data
	{
		u64 light_count{ 0ull };

		// Begin calculating cascades
		const auto& near_clip = camera.zNear;
		const auto& far_clip = camera.zFar;
		f32 clip_range = far_clip - near_clip;

		f32 min_z = near_clip;
		f32 max_z = near_clip + clip_range;

		f32 range = max_z - min_z;
		f32 ratio = max_z / min_z;

		auto& culler_group = m_shadowCullers[ERenderGeometryCullerType::eCascadedShadow];

		auto view = registry->view<FTransformComponent, FBaseComponent, FDirectionalLightComponent>();
		for (auto [entity, ltransform, base, light] : view.each())
		{
			// Reset shadow index
			light.shadowIndex = -1;

			// Calculate shadows
			std::array<f32, cascade_shadow_map_cascade_count> cascadeSplits;
			if (light.castShadows && base.enabled && world_stats.cascadedShadowCount < max_directional_light_shadow_count)
			{
				FCascadedShadowCommit shadow_commit{};

				auto& culler_list = culler_group[world_stats.cascadedShadowCount];
				shadow_commit.cameraID = culler_list.front(); // Adding first camera id

				// Calculate split depths based on view camera frustum
				// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
				for (u32 i = 0; i < cascade_shadow_map_cascade_count; i++)
				{
					f32 p = utl::cast<f32>(i + 1) / utl::cast<f32>(cascade_shadow_map_cascade_count);
					f32 log = min_z * glm::pow(ratio, p);
					f32 uniform = min_z + range * p;
					f32 d = cascade_split_lambda * (log - uniform) + uniform;
					cascadeSplits[i] = (d - near_clip) / clip_range;
				}

				// Calculate orthographic projection matrix for each cascade
				f32 lastSplitDist = 0.0;
				for (u32 i = 0; i < cascade_shadow_map_cascade_count; i++)
				{
					auto& splitDist = cascadeSplits[i];

					std::array<glm::vec4, 8> frustumCorners =
					{
						// Near face
						glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f),
						glm::vec4(1.0f, -1.0f, 0.0f, 1.0f),
						glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
						glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f),
						
						// Far face
						glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
						glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),
						glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
						glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f),
					};
	
					// Project frustum corners into world space
					for (auto& corner : frustumCorners)
					{
						glm::vec4 invCorner = render_world->getMatrix(camera.matricesOffset + render::inv_view_projection_matrix_offset) * corner;
						corner = invCorner / invCorner.w;
					}

					for (u32 j = 0; j < 4; ++j)
					{
						glm::vec4 dist = frustumCorners[j + 4] - frustumCorners[j];
						frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
						frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
					}

					// Get frustum center
					glm::vec3 frustumCenter = glm::vec3(0.0f);
					for (auto& corner : frustumCorners)
						frustumCenter += glm::vec3(corner);
					frustumCenter /= frustumCorners.size();

					volatile f32 radius{ 0.0f };
					for (auto& corner : frustumCorners)
					{
						f32 distance = glm::length(glm::vec3(corner) - frustumCenter);
						radius = (glm::max)(radius, distance);
					}
					radius = glm::ceil(radius * 16.0f) / 16.0f;

					glm::vec3 maxExtents = glm::vec3(radius);
					glm::vec3 minExtents = -maxExtents;

					glm::vec3 lightDir = glm::normalize(ltransform.grotation * glm::vec3(0.f, 0.f, -1.f));
					glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * radius, frustumCenter, glm::vec3(0.f, 1.f, 0.f));
					glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, minExtents.z - maxExtents.z, maxExtents.z - minExtents.z);

					auto shadowMatrix = lightOrthoMatrix * lightViewMatrix;
					glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
					shadowOrigin = shadowMatrix * shadowOrigin;
					f32 storedW = shadowOrigin.w;
					shadowOrigin = shadowOrigin * utl::cast<f32>(cascade_shadow_map_resolution) / 2.0f;
					glm::vec4 roundedOrigin = glm::round(shadowOrigin);
					glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
					roundOffset = roundOffset * 2.0f / utl::cast<f32>(cascade_shadow_map_resolution);
					roundOffset.z = 0.0f;
					roundOffset.w = 0.0f;
					glm::mat4 lightProjectionMatrix = lightOrthoMatrix;
					lightProjectionMatrix[3] += roundOffset;

					// Store split distance and matrix in cascade
					shadow_commit.farClip = far_clip;
					shadow_commit.splitDepths[i] = (near_clip + splitDist * clip_range);

					lastSplitDist = cascadeSplits[i];

					auto& prev_view = render_world->getPrevMatrix(culler_list[i] + render::view_matrix_offset);
					auto& prev_projection = render_world->getPrevMatrix(shadow_commit.cameraID + render::projection_matrix_offset);
					render_world->updateCamera(culler_list[i], lightViewMatrix, prev_view, lightProjectionMatrix, prev_projection, glm::vec2(cascade_shadow_map_resolution, cascade_shadow_map_resolution));
				}

				shadow_commit.index = light_count;

				// Set shadow index and increment it
				light.shadowIndex = world_stats.cascadedShadowCount;
				render_world->addShadow(shadow_commit);
			}

			++light_count;
		}
	}



	// Calculate omni shadows
	{
		std::array<glm::mat4, 6u> matrices{};
		auto& culler_group = m_shadowCullers[ERenderGeometryCullerType::eOmniShadow];

		u64 light_count{ 0ull };
		auto view = registry->view<FTransformComponent, FBaseComponent, FPointLightComponent>();
		for (auto [entity, ltransform, base, light] : view.each())
		{
			light.shadowIndex = -1;

			//light.castShadows = light_count % 2;
			if (light.castShadows && base.enabled && world_stats.omniShadowCount < max_point_light_shadow_count)
			{
				FOmniShadowCommit shadow_commit{};

				auto& culler_list = culler_group[world_stats.omniShadowCount];
				shadow_commit.cameraID = culler_list.front();

				glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, light.radius / 2.f);

				matrices[0] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); // POSITIVE_X
				matrices[1] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); // NEGATIVE_X
				matrices[2] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)); // POSITIVE_Y
				matrices[3] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));	// NEGATIVE_Y
				matrices[4] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));	// POSITIVE_Z
				matrices[5] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));	// NEGATIVE_Z

				shadow_commit.farPlane = light.radius;
				shadow_commit.index = light_count;

				light.shadowIndex = world_stats.omniShadowCount;

				for (u64 idx = 0ull; idx < matrices.size(); ++idx)
				{
					auto& view = matrices[idx];
					auto& prev_view = render_world->getPrevMatrix(shadow_commit.cameraID + render::view_matrix_offset);
					auto& prev_projection = render_world->getPrevMatrix(shadow_commit.cameraID + render::projection_matrix_offset);
					render_world->updateCamera(culler_list[idx], view, prev_view, shadowProj, prev_projection, glm::vec2(point_light_shadow_map_resolution, point_light_shadow_map_resolution), 0.f, light.radius / 2.f);
				}

				render_world->addShadow(shadow_commit);
			}

			++light_count;
		}
	}



	// Calculate spot shadows
	{
		auto& culler_group = m_shadowCullers[ERenderGeometryCullerType::eSpotShadow];

		u64 light_count{ 0ull };
		auto view = registry->view<FTransformComponent, FBaseComponent, FSpotLightComponent>();
		for (auto [entity, ltransform, base, light] : view.each())
		{
			light.shadowIndex = -1;

			if (light.castShadows && base.enabled && world_stats.spotShadowCount < max_spot_light_count)
			{
				auto& culler_list = culler_group[world_stats.spotShadowCount];

				FSpotShadowCommit shadow_commit{};
				shadow_commit.cameraID = culler_list.front();

				auto light_dir = light.toTarget ? light.target : glm::normalize(ltransform.grotation * glm::vec3(0.f, 0.f, -1.f));
				glm::mat4 shadowProj = glm::perspective(light.outerAngle * 2.f, 1.0f, 0.1f, 64.f);
				glm::mat4 shadowView = glm::lookAt(ltransform.gposition, ltransform.gposition + light_dir, glm::vec3(0.0f, 1.0f, 0.0f));
				shadow_commit.index = light_count;

				light.shadowIndex = world_stats.spotShadowCount;
				render_world->addShadow(shadow_commit);
				
				auto& prev_view = render_world->getPrevMatrix(shadow_commit.cameraID + render::view_matrix_offset);
				auto& prev_projection = render_world->getPrevMatrix(shadow_commit.cameraID + render::projection_matrix_offset);
				render_world->updateCamera(shadow_commit.cameraID, shadowView, prev_view, shadowProj, prev_projection, glm::vec2(spot_light_shadow_map_resolution, spot_light_shadow_map_resolution), 0.f, 64.f);
			}

			++light_count;
		}
	}
}
