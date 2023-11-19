#include "ShadowsSystem.h"

#include <world/SceneGraph.h>

#include <world/ecs/components/BaseComponent.h>
#include <world/ecs/components/TransformComponent.h>
#include <world/ecs/components/DirectionalLightComponent.h>
#include <world/ecs/components/PointLightComponent.h>
#include <world/ecs/components/SpotLightComponent.h>
#include <world/ecs/components/MeshComponent.h>

using namespace engine::render;
using namespace engine::world::ecs;

constexpr const float cascade_split_lambda = 0.985f;
constexpr const float cascade_split_overlap = 0.9f;

void get_primitives_in_shadow_view(const std::unique_ptr<entt::registry>& registry, const FFrustum& frustum, std::vector<FGeometryDrawData>& geometry_list)
{
	auto meshes = registry->view<FTransformComponent, FBaseComponent, FMeshComponent>();
	for (auto [entity, mtransform, base, mesh] : meshes.each())
	{
		if (!base.visible || !mesh.castShadows)
			continue;

		//glm::vec4 clip_space_pos = shadow_commit.cascadeViewProj[i] * mtransform.gmodel * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		//float clip_space_z = clip_space_pos.z / clip_space_pos.w;
		//float world_z = render_packet->camera.nearp + (render_packet->camera.farp - render_packet->camera.nearp) * ((clip_space_z + 1.0f) / 2.0f);

		for (const auto& primitive : mesh.primitives)
		{
			bool is_in_light_view = frustum.checkBox(mtransform.gposition + primitive.dims.min * mtransform.gscale, mtransform.gposition + primitive.dims.max * mtransform.gscale);
			if (is_in_light_view)
			{
				auto& lod = primitive.lods[0ull];

				auto& draw_data = geometry_list.emplace_back();
				draw_data.vboId = primitive.vboId;
				draw_data.range.beginIndex = lod.beginIndex;
				draw_data.range.indexCount = lod.indexCount;
				draw_data.range.beginVertex = lod.beginVertex;
				draw_data.range.vertexCount = lod.indexCount;
			}
		}
	}
}

CShadowsSystem::CShadowsSystem(CSceneGraph* scenegraph, IRenderInterface* render_interface)
{
	m_pSceneGraph = scenegraph;
	m_pRenderInterface = render_interface;
}

void CShadowsSystem::__create()
{

}

void CShadowsSystem::__update(float fDt)
{
	auto& registry = m_pSceneGraph->getRegistry();
	auto& render_packet = m_pRenderInterface->getRenderPacket();

	// Calculate cascades shadow data
	{
		// Begin calculating cascades
		const auto& near_clip = render_packet->camera.nearp;
		const auto& far_clip = render_packet->camera.farp;
		float clip_range = far_clip - near_clip;

		float min_z = near_clip;
		float max_z = near_clip + clip_range;

		float range = max_z - min_z;
		float ratio = max_z / min_z;

		uint32_t shadow_index{ 0u };
		auto view = registry->view<FTransformComponent, FBaseComponent, FDirectionalLightComponent>();
		for (auto [entity, ltransform, base, light] : view.each())
		{
			// Reset shadow index
			light.shadowIndex = -1;

			auto& geometry_lists = render_packet->directionalLights.shadows.draws.at(shadow_index);

			// Calculate shadows
			std::array<float, cascade_shadow_map_cascade_count> cascadeSplits;
			if (light.castShadows && base.enabled && shadow_index < max_directional_light_shadow_count)
			{
				auto& shadow_commit = render_packet->directionalLights.shadows.commits.at(shadow_index);

				// Calculate split depths based on view camera frustum
				// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
				for (uint32_t i = 0; i < cascade_shadow_map_cascade_count; i++)
				{
					float p = static_cast<float>(i + 1) / static_cast<float>(cascade_shadow_map_cascade_count);
					float log = min_z * glm::pow(ratio, p);
					float uniform = min_z + range * p;
					float d = cascade_split_lambda * (log - uniform) + uniform;
					cascadeSplits[i] = (d - near_clip) / clip_range;
				}

				// Calculate orthographic projection matrix for each cascade
				float lastSplitDist = 0.0;
				for (uint32_t i = 0; i < cascade_shadow_map_cascade_count; i++)
				{
					auto& splitDist = cascadeSplits[i];

					std::array<glm::vec4, 8> frustumCorners =
					{
						//Near face
						glm::vec4{  1.0f,  1.0f, -1.0f, 1.0f },
						glm::vec4{ -1.0f,  1.0f, -1.0f, 1.0f },
						glm::vec4{  1.0f, -1.0f, -1.0f, 1.0f },
						glm::vec4{ -1.0f, -1.0f, -1.0f, 1.0f },

						//Far face
						glm::vec4{  1.0f,  1.0f, 1.0f, 1.0f },
						glm::vec4{ -1.0f,  1.0f, 1.0f, 1.0f },
						glm::vec4{  1.0f, -1.0f, 1.0f, 1.0f },
						glm::vec4{ -1.0f, -1.0f, 1.0f, 1.0f },
					};

					// Project frustum corners into world space
					glm::mat4 invCam = glm::inverse(render_packet->camera.projection * render_packet->camera.view);
					for (auto& corner : frustumCorners)
					{
						glm::vec4 invCorner = invCam * corner;
						corner = invCorner / invCorner.w;
					}

					for (uint32_t j = 0; j < 4; ++j)
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

					float radius = 0.0f;
					for (auto& corner : frustumCorners)
					{
						float distance = glm::length(glm::vec3(corner) - frustumCenter);
						radius = (glm::max)(radius, distance);
					}
					radius = glm::ceil(radius * 16.0f) / 16.0f;

					glm::vec3 maxExtents = glm::vec3(radius);
					glm::vec3 minExtents = -maxExtents;

					glm::vec3 lightDir = glm::normalize(glm::toQuat(ltransform.gmodel) * glm::vec3(0.f, 0.f, 1.f));
					glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, render_packet->camera.up);
					glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, minExtents.z, maxExtents.z - minExtents.z);

					auto shadowMatrix = lightOrthoMatrix * lightViewMatrix;
					glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
					shadowOrigin = shadowMatrix * shadowOrigin;
					float storedW = shadowOrigin.w;
					shadowOrigin = shadowOrigin * static_cast<float>(cascade_shadow_map_resolution) / 2.0f;
					glm::vec4 roundedOrigin = glm::round(shadowOrigin);
					glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
					roundOffset = roundOffset * 2.0f / static_cast<float>(cascade_shadow_map_resolution);
					roundOffset.z = 0.0f;
					roundOffset.w = 0.0f;
					glm::mat4 lightProjectionMatrix = lightOrthoMatrix;
					lightProjectionMatrix[3] += roundOffset;

					// Store split distance and matrix in cascade
					shadow_commit.farClip = far_clip;
					shadow_commit.splitDepths[i] = (near_clip + splitDist * clip_range) * -1.0f;
					shadow_commit.cascadeViewProj[i] = lightProjectionMatrix * lightViewMatrix;

					lastSplitDist = cascadeSplits[i];

					FFrustum frustum{};
					frustum.update(lightViewMatrix, lightProjectionMatrix);

					auto& geometry_list = geometry_lists.at(i);

					// Prepare static mesh data
					get_primitives_in_shadow_view(registry, frustum, geometry_list);
				}

				// Set shadow index and increment it
				light.shadowIndex = shadow_index;
				shadow_index++;
			}
		}
	}



	// Calculate omni shadows
	{
		uint32_t shadow_index{ 0u };

		auto lights = registry->view<FTransformComponent, FBaseComponent, FPointLightComponent>();
		for (auto [entity, ltransform, base, light] : lights.each())
		{
			light.shadowIndex = -1;

			if (light.castShadows && base.enabled && shadow_index < max_point_light_shadow_count)
			{
				auto& geometry_lists = render_packet->pointLights.shadows.draws.at(shadow_index);
				auto& shadow_commit = render_packet->pointLights.shadows.commits.at(shadow_index);

				uint32_t array_shift = shadow_index * 6u;

				glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, light.radius);
				shadow_commit.viewProj[0ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); // POSITIVE_X
				shadow_commit.viewProj[1ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); // NEGATIVE_X
				shadow_commit.viewProj[2ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)); // POSITIVE_Y
				shadow_commit.viewProj[3ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));	// NEGATIVE_Y
				shadow_commit.viewProj[4ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));	// POSITIVE_Z
				shadow_commit.viewProj[5ull] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));	// NEGATIVE_Z

				// Calculate geometry to draw
				for (uint32_t face = 0u; face < 6u; face++)
				{
					auto& geometry_list = geometry_lists.at(face);

					FFrustum frustum{};
					frustum.update(shadow_commit.viewProj.at(face), shadowProj);

					get_primitives_in_shadow_view(registry, frustum, geometry_list);
				}
			}
		}
	}



	// Calculate spot shadows
	{
		uint32_t shadow_index{ 0u };

		auto lights = registry->view<FTransformComponent, FBaseComponent, FSpotLightComponent>();
		for (auto [entity, ltransform, base, light] : lights.each())
		{
			light.shadowIndex = -1;

			if (light.castShadows && base.enabled && shadow_index < max_spot_light_count)
			{
				auto& geometry_list = render_packet->spotLights.shadows.draws.at(shadow_index);
				auto& shadow_commit = render_packet->spotLights.shadows.commits.at(shadow_index);

				auto light_dir = light.toTarget ? light.target : glm::normalize(glm::toQuat(ltransform.gmodel) * glm::vec3(0.f, 0.f, 1.f));
				glm::mat4 shadowProj = glm::perspective(light.outerAngle * 2.f, 1.0f, 0.1f, 64.f);
				glm::mat4 shadowView = glm::lookAt(ltransform.gposition, ltransform.gposition + light_dir, glm::vec3(0.0f, 1.0f, 0.0f));
				shadow_commit.viewProj = shadowProj * shadowView;

				FFrustum frustum{};
				frustum.update(shadowView, shadowProj);

				get_primitives_in_shadow_view(registry, frustum, geometry_list);
			}
		}
	}
}
