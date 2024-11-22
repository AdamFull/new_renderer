#include "ShadowsSystem.h"

#include <engine/world/EntityManager.h>

#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>
#include <engine/world/ecs/components/DirectionalLightComponent.h>
#include <engine/world/ecs/components/PointLightComponent.h>
#include <engine/world/ecs/components/SpotLightComponent.h>
#include <engine/world/ecs/components/MeshComponent.h>

#include <tracy/Tracy.hpp>

#include <glm/gtc/matrix_transform.hpp>

using namespace engine::render;
using namespace engine::world::ecs;

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			constexpr const f32 cascade_split_lambda = 0.85f;

			CShadowsSystem::CShadowsSystem(render::IRenderer& renderer) :
				render_world{ renderer.GetRenderWorld() }
			{
			}

			void CShadowsSystem::__Create()
			{
			}

			void CShadowsSystem::__Update(f32 fDt)
			{
				ZoneScoped;

				// TODO: get current player camera?
				static const uint32_t main_camera_id{ 0u };
				auto& camera = render_world.GetRenderCamera(main_camera_id);

				u64 light_count{ 0ull };

				// Calculate cascades shadow data
				{
					uint32_t dir_light_count{ 0u };

					// Begin calculating cascades
					const auto& near_clip = camera.z_near;
					const auto& far_clip = camera.z_far;
					f32 clip_range = far_clip - near_clip;

					f32 min_z = near_clip;
					f32 max_z = near_clip + clip_range;

					f32 range = max_z - min_z;
					f32 ratio = max_z / min_z;

					auto view = EntityManager::View<FTransformComponent, FBaseComponent, FDirectionalLightComponent>();
					for (auto [entity, ltransform, base, light] : view.each())
					{
						// Reset shadow index
						light.shadowIndex = -1;

						// Calculate shadows
						std::array<f32, shlib::cascade_shadow_map_cascade_count> cascadeSplits;
						if (light.castShadows && base.enabled && dir_light_count < shlib::max_directional_light_shadow_count)
						{
							shlib::CascadedShadowCommit shadow_commit{};
							shadow_commit.cameraID = light.shadow_casters.front(); // Adding first camera id

							// Calculate split depths based on view camera frustum
							// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
							for (u32 i = 0; i < shlib::cascade_shadow_map_cascade_count; i++)
							{
								f32 p = utl::cast<f32>(i + 1) / utl::cast<f32>(shlib::cascade_shadow_map_cascade_count);
								f32 log = min_z * glm::pow(ratio, p);
								f32 uniform = min_z + range * p;
								f32 d = cascade_split_lambda * (log - uniform) + uniform;
								cascadeSplits[i] = (d - near_clip) / clip_range;
							}

							// Calculate orthographic projection matrix for each cascade
							f32 lastSplitDist = 0.0;
							for (u32 i = 0; i < shlib::cascade_shadow_map_cascade_count; i++)
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
									glm::vec4 invCorner = render_world.GetCameraMatrix(main_camera_id, shlib::inv_view_projection_matrix_offset) * corner;
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
								shadowOrigin = shadowOrigin * utl::cast<f32>(shlib::cascade_shadow_map_resolution) / 2.0f;
								glm::vec4 roundedOrigin = glm::round(shadowOrigin);
								glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
								roundOffset = roundOffset * 2.0f / utl::cast<f32>(shlib::cascade_shadow_map_resolution);
								roundOffset.z = 0.0f;
								roundOffset.w = 0.0f;
								glm::mat4 lightProjectionMatrix = lightOrthoMatrix;
								lightProjectionMatrix[3] += roundOffset;

								// Store split distance and matrix in cascade
								shadow_commit.farClip = far_clip;
								shadow_commit.splitDepths[i] = (near_clip + splitDist * clip_range);

								lastSplitDist = cascadeSplits[i];

								render_world.UpdateRenderCamera(light.shadow_casters[i], lightViewMatrix, lightProjectionMatrix, shlib::cascade_shadow_map_resolution, shlib::cascade_shadow_map_resolution);
							}

							shadow_commit.index = light_count;

							// Set shadow index and increment it
							light.shadowIndex = dir_light_count++;
							render_world.AddShadow(shadow_commit);
						}

						++light_count;
					}
				}
				
				
				
				// Calculate omni shadows
				{
					uint32_t point_light_count{ 0u };

					std::array<glm::mat4, 6u> matrices{};
				
					auto view = EntityManager::View<FTransformComponent, FBaseComponent, FPointLightComponent>();
					for (auto [entity, ltransform, base, light] : view.each())
					{
						light.shadowIndex = -1;
				
						//light.castShadows = light_count % 2;
						if (light.castShadows && base.enabled && point_light_count < shlib::max_point_light_shadow_count)
						{
							shlib::OmniShadowCommit shadow_commit{};
				
							shadow_commit.cameraID = light.shadow_casters.front();
				
							glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, light.radius / 2.f);
				
							matrices[0] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); // POSITIVE_X
							matrices[1] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); // NEGATIVE_X
							matrices[2] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)); // POSITIVE_Y
							matrices[3] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));	// NEGATIVE_Y
							matrices[4] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));	// POSITIVE_Z
							matrices[5] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));	// NEGATIVE_Z
				
							shadow_commit.farPlane = light.radius;
							shadow_commit.index = light_count;
				
							light.shadowIndex = point_light_count++;
				
							for (u64 idx = 0ull; idx < matrices.size(); ++idx)
							{
								auto& view = matrices[idx];
								render_world.UpdateRenderCamera(light.shadow_casters[idx], view, shadowProj, shlib::point_light_shadow_map_resolution, shlib::point_light_shadow_map_resolution);
							}
				
							render_world.AddShadow(shadow_commit);
						}
				
						++light_count;
					}
				}
				
				
				
				// Calculate spot shadows
				{
					uint32_t spot_light_count{ 0u };
				
					auto view = EntityManager::View<FTransformComponent, FBaseComponent, FSpotLightComponent>();
					for (auto [entity, ltransform, base, light] : view.each())
					{
						light.shadowIndex = -1;
				
						if (light.castShadows && base.enabled && spot_light_count < shlib::max_spot_light_count)
						{
							shlib::SpotShadowCommit shadow_commit{};
							shadow_commit.cameraID = light.shadow_caster;
				
							auto light_dir = light.toTarget ? light.target : glm::normalize(ltransform.grotation * glm::vec3(0.f, 0.f, -1.f));
							glm::mat4 shadowProj = glm::perspective(light.outerAngle * 2.f, 1.0f, 0.1f, 64.f);
							glm::mat4 shadowView = glm::lookAt(ltransform.gposition, ltransform.gposition + light_dir, glm::vec3(0.0f, 1.0f, 0.0f));
							shadow_commit.index = light_count;
				
							light.shadowIndex = spot_light_count++;
							render_world.AddShadow(shadow_commit);
							
							render_world.UpdateRenderCamera(shadow_commit.cameraID, shadowView, shadowProj, shlib::spot_light_shadow_map_resolution, shlib::spot_light_shadow_map_resolution);
						}
				
						++light_count;
					}
				}
			}

		}
	}
}