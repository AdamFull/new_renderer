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
	TRACY_PROFILE_FUNCTION();
}

void CShadowsSystem::__update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = m_pSceneGraph->getRegistry();
	auto& render_packet = m_pRenderInterface->getRenderPacket();
	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& world_stats = render_world->getStats();

	// Calculate cascades shadow data
	{
		world_stats.cascadedShadowCount = 0ull;

		// Begin calculating cascades
		const auto& near_clip = render_packet->camera.nearp;
		const auto& far_clip = render_packet->camera.farp;
		f32 clip_range = far_clip - near_clip;

		f32 min_z = near_clip;
		f32 max_z = near_clip + clip_range;

		f32 range = max_z - min_z;
		f32 ratio = max_z / min_z;

		auto view = registry->view<FTransformComponent, FBaseComponent, FDirectionalLightComponent>();
		for (auto [entity, ltransform, base, light] : view.each())
		{
			// Reset shadow index
			light.shadowIndex = -1;

			// Calculate shadows
			stl::array<f32, cascade_shadow_map_cascade_count> cascadeSplits;
			if (light.castShadows && base.enabled && world_stats.cascadedShadowCount < max_directional_light_shadow_count)
			{
				FCascadedShadowCommit shadow_commit{};

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

					stl::array<glm::vec4, 8> frustumCorners =
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
						glm::vec4 invCorner = render_packet->camera.invViewProjection * corner;
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
					shadow_commit.cascadeViewProj[i] = lightProjectionMatrix * lightViewMatrix;

					lastSplitDist = cascadeSplits[i];

					render_world->updateFrustum(light.frustumIDs[i], lightViewMatrix, lightProjectionMatrix);
				}

				// Set shadow index and increment it
				light.shadowIndex = world_stats.cascadedShadowCount;
				render_world->addShadow(shadow_commit);
			}
		}
	}



	// Calculate omni shadows
	{
		world_stats.omniShadowCount = 0ull;
		auto view = registry->view<FTransformComponent, FBaseComponent, FPointLightComponent>();
		for (auto [entity, ltransform, base, light] : view.each())
		{
			light.shadowIndex = -1;

			if (light.castShadows && base.enabled && world_stats.omniShadowCount < max_point_light_shadow_count)
			{
				glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, light.radius);

				FOmniShadowCommit shadow_commit{};
				shadow_commit.viewProj[0ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); // POSITIVE_X
				shadow_commit.viewProj[1ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); // NEGATIVE_X
				shadow_commit.viewProj[2ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)); // POSITIVE_Y
				shadow_commit.viewProj[3ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));	// NEGATIVE_Y
				shadow_commit.viewProj[4ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));	// POSITIVE_Z
				shadow_commit.viewProj[5ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));	// NEGATIVE_Z

				light.shadowIndex = world_stats.omniShadowCount;

				for (auto& view : shadow_commit.viewProj)
				{
					render_world->updateFrustum(light.frustumID, view, shadowProj);
					view = shadowProj * view;
				}

				render_world->addShadow(shadow_commit);
			}
		}
	}



	// Calculate spot shadows
	{
		world_stats.spotShadowCount = 0ull;
		auto view = registry->view<FTransformComponent, FBaseComponent, FSpotLightComponent>();
		for (auto [entity, ltransform, base, light] : view.each())
		{
			light.shadowIndex = -1;

			if (light.castShadows && base.enabled && world_stats.spotShadowCount < max_spot_light_count)
			{
				FSpotShadowCommit shadow_commit{};

				auto light_dir = light.toTarget ? light.target : glm::normalize(ltransform.grotation * glm::vec3(0.f, 0.f, -1.f));
				glm::mat4 shadowProj = glm::perspective(light.outerAngle * 2.f, 1.0f, 0.1f, 64.f);
				glm::mat4 shadowView = glm::lookAt(ltransform.gposition, ltransform.gposition + light_dir, glm::vec3(0.0f, 1.0f, 0.0f));
				shadow_commit.viewProj = shadowProj * shadowView;

				light.shadowIndex = world_stats.spotShadowCount;
				render_world->addShadow(shadow_commit);

				render_world->updateFrustum(light.frustumID, shadowView, shadowProj);
			}
		}
	}
}
