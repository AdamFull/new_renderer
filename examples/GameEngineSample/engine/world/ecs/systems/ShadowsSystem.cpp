#include "ShadowsSystem.h"

#include "../../EntityManager.h"

#include "../components/BaseComponent.h"
#include "../components/TransformComponent.h"
#include "../components/DirectionalLightComponent.h"
#include "../components/PointLightComponent.h"
#include "../components/SpotLightComponent.h"
#include "../components/MeshComponent.h"

#include <edge/util/utl_profiler.h>

#include <glm/gtc/matrix_transform.hpp>

#include "../../../rendering/Globals.h"

using namespace engine::render;

namespace engine::world::ecs {
	constexpr const float cascade_split_lambda = 0.85f;

	ShadowsSystem::ShadowsSystem(render::IRenderer& renderer) :
		render_world{ renderer.get_render_world() } {
	}

	void ShadowsSystem::__create() {
	}

	void ShadowsSystem::__update(float fDt) {
		EDGE_PROFILE_ZONE_SCOPED;

		// TODO: get current player camera?
		auto& camera = render_world.get_render_camera(render::main_camera_index);

		int32_t light_count{ 0 };

		// Calculate cascades shadow data
		{
			uint32_t dir_light_count{ 0u };

			// Begin calculating cascades
			const auto& near_clip = camera.z_near;
			const auto& far_clip = camera.z_far;
			float clip_range = far_clip - near_clip;

			float min_z = near_clip;
			float max_z = near_clip + clip_range;

			float range = max_z - min_z;
			float ratio = max_z / min_z;

			auto view = EntityManager::view<TransformComponent, BaseComponent, DirectionalLightComponent>();
			for (auto [entity, ltransform, base, light] : view.each()) {
				// Reset shadow index
				light.shadowIndex = -1;

				// Calculate shadows
				float cascade_splits[constants::cascade_shadow_map_cascade_count] = {};
				if (light.castShadows && base.enabled && dir_light_count < constants::max_directional_light_shadow_count) {
					interop::CascadedShadowCommit shadow_commit{};

					// Calculate split depths based on view camera frustum
					// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
					for (int32_t i = 0; i < constants::cascade_shadow_map_cascade_count; i++) {
						float p = static_cast<float>(i + 1) / static_cast<float>(constants::cascade_shadow_map_cascade_count);
						float log = min_z * glm::pow(ratio, p);
						float uniform = min_z + range * p;
						float d = cascade_split_lambda * (log - uniform) + uniform;
						cascade_splits[i] = (d - near_clip) / clip_range;
					}

					std::array<glm::vec3, 8> frustum_corners_ws{
						glm::vec3(-1.0f, -1.0f, 0.0f),
						glm::vec3(-1.0f, 1.0f, 0.0f),
						glm::vec3(1.0f, 1.0f, 0.0f),
						glm::vec3(1.0f, -1.0f, 0.0f),

						glm::vec3(-1.0f, -1.0f, 1.0f),
						glm::vec3(-1.0f, 1.0f, 1.0f),
						glm::vec3(1.0f, 1.0f, 1.0f),
						glm::vec3(1.0f, -1.0f, 1.0f)
					};

					glm::mat4 inverse_vp = render_world.get_camera_matrix(render::main_camera_index, constants::inv_view_projection_matrix_offset);

					// Project frustum corners into world space
					for (auto& corner : frustum_corners_ws) {
						glm::vec4 inv_corner = inverse_vp * glm::vec4(corner, 1.0f);
						corner = glm::vec3(inv_corner) / inv_corner.w;
					}

					glm::vec3 light_dir = glm::normalize(glm::rotate(ltransform.lrotation, glm::vec3(0.0f, 0.0f, -1.0f)));
					glm::vec3 up_vector = glm::abs(glm::dot(light_dir, glm::vec3(0, 1, 0))) > 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
					glm::mat4 light_view = glm::lookAt(glm::vec3(0.0f), light_dir, up_vector);

					// Calculate orthographic projection matrix for each cascade
					float last_split_dist = 0.0;
					for (int32_t i = 0; i < constants::cascade_shadow_map_cascade_count; i++) {
						float current_split = cascade_splits[i];

						// Compute the frustum corners for the cascade in view space
						std::array<glm::vec3, 8> frustum_corners_ls{};
						for (int32_t j = 0; j < 4; ++j) {
							frustum_corners_ls[j] = light_view * glm::vec4(glm::mix(frustum_corners_ws[j], frustum_corners_ws[j + 4], last_split_dist), 1.0f);
							frustum_corners_ls[j + 4] = light_view * glm::vec4(glm::mix(frustum_corners_ws[j], frustum_corners_ws[j + 4], current_split), 1.0f);
						}

						glm::vec3 center = glm::vec3(0.0f);
						for (auto& corner : frustum_corners_ls) {
							center += corner;
						}
						center *= 0.125f;

						//Create a bounding sphere to maintain aspect in projection to avoid flickering when rotating
						float radius = 0;
						for (auto& corner : frustum_corners_ls) {
							radius = glm::max(radius, glm::length(corner - center));
						}
						radius = glm::ceil(radius * 16.0f) / 16.0f;

						glm::vec3 min_extents = center - glm::vec3(radius);
						glm::vec3 max_extents = center + glm::vec3(radius);

						glm::vec3 extents = max_extents - min_extents;
						glm::vec3 texel_size = extents / glm::vec3(constants::cascade_shadow_map_resolution);
						min_extents = glm::floor(min_extents / texel_size) * texel_size;
						max_extents = glm::ceil(max_extents / texel_size) * texel_size;
						center = (min_extents + max_extents) * 0.5f;

						min_extents.z = std::numeric_limits<float>::max();
						max_extents.z = std::numeric_limits<float>::lowest();

						for (const auto& corner : frustum_corners_ls) {
							min_extents.z = glm::min(min_extents.z, corner.z);
							max_extents.z = glm::max(max_extents.z, corner.z);
						}

						float depth_bias = 10.0f;
						glm::mat4 light_projection = glm::ortho(min_extents.x, max_extents.x, min_extents.y, max_extents.y, min_extents.z - depth_bias, max_extents.z - min_extents.z);

						render_world.update_render_camera(light.shadow_casters[i], light_view, light_projection, false, constants::cascade_shadow_map_resolution, constants::cascade_shadow_map_resolution);

						auto& camera = render_world.get_render_camera(light.shadow_casters[i]);
						shadow_commit.far_clip = far_clip;
						shadow_commit.split_depths[i] = (near_clip + current_split * clip_range);
						shadow_commit.matrix_offsets[i] = camera.matrices_offset;

						// Update meshes
						auto mview = EntityManager::view<TransformComponent, BaseComponent, MeshComponent>();
						for (auto [mentity, mtransform, mbase, mesh_component] : mview.each()) {
							if (!base.visible) {
								continue;
							}

							render_world.add_geometry_draw(light.shadow_casters[i], mesh_component.mesh);
						}

						last_split_dist = current_split;
					}

					shadow_commit.index = light_count;

					// Set shadow index and increment it
					light.shadowIndex = dir_light_count++;
					render_world.add_shadow(shadow_commit);
				}

				++light_count;
			}
		}

#if 0

		// Calculate omni shadows
		{
			uint32_t point_light_count{ 0u };

			std::array<glm::mat4, 6u> matrices{};

			auto view = EntityManager::view<TransformComponent, BaseComponent, PointLightComponent>();
			for (auto [entity, ltransform, base, light] : view.each()) {
				light.shadowIndex = -1;

				//light.castShadows = light_count % 2;
				if (light.castShadows && base.enabled && point_light_count < constants::max_point_light_shadow_count) {
					interop::OmniShadowCommit shadow_commit{};

					glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, light.radius);

					matrices[0] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); // POSITIVE_X
					matrices[1] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); // NEGATIVE_X
					matrices[2] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)); // POSITIVE_Y
					matrices[3] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));	// NEGATIVE_Y
					matrices[4] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));	// POSITIVE_Z
					matrices[5] = glm::lookAt(ltransform.gposition, ltransform.gposition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));	// NEGATIVE_Z

					shadow_commit.far_plane = light.radius;
					shadow_commit.index = light_count;

					light.shadowIndex = point_light_count++;

					for (uint64_t idx = 0ull; idx < matrices.size(); ++idx) {
						auto& view = matrices[idx];
						render_world.update_render_camera(light.shadow_casters[idx], view, shadowProj, true, constants::point_light_shadow_map_resolution, constants::point_light_shadow_map_resolution);

						auto& camera = render_world.get_render_camera(light.shadow_casters[idx]);
						shadow_commit.matrix_offsets[idx] = camera.matrices_offset;

						// Update meshes
						auto mview = EntityManager::view<TransformComponent, BaseComponent, MeshComponent>();
						for (auto [mentity, mtransform, mbase, mesh_component] : mview.each()) {
							if (!base.visible) {
								continue;
							}

							render_world.add_geometry_draw(light.shadow_casters[idx], mesh_component.mesh);
						}
					}

					render_world.add_shadow(shadow_commit);
				}

				++light_count;
			}
		}



		// Calculate spot shadows
		{
			uint32_t spot_light_count{ 0u };

			auto view = EntityManager::view<TransformComponent, BaseComponent, SpotLightComponent>();
			for (auto [entity, ltransform, base, light] : view.each()) {
				light.shadowIndex = -1;

				if (light.castShadows && base.enabled && spot_light_count < constants::max_spot_light_count) {
					interop::SpotShadowCommit shadow_commit{};

					auto& camera = render_world.get_render_camera(light.shadow_caster);
					shadow_commit.matrix_offset = camera.matrices_offset;

					auto light_dir = light.toTarget ? light.target : glm::normalize(ltransform.grotation * glm::vec3(0.f, 0.f, -1.f));
					glm::mat4 shadowProj = glm::perspective(light.outerAngle * 2.f, 1.0f, 0.1f, 64.f);
					glm::mat4 shadowView = glm::lookAt(ltransform.gposition, ltransform.gposition + light_dir, glm::vec3(0.0f, 1.0f, 0.0f));
					shadow_commit.index = light_count;

					light.shadowIndex = spot_light_count++;
					render_world.add_shadow(shadow_commit);

					render_world.update_render_camera(light.shadow_caster, shadowView, shadowProj, true, constants::spot_light_shadow_map_resolution, constants::spot_light_shadow_map_resolution);

					// Update meshes
					auto mview = EntityManager::view<TransformComponent, BaseComponent, MeshComponent>();
					for (auto [mentity, mtransform, mbase, mesh_component] : mview.each()) {
						if (!base.visible)
							continue;

						render_world.add_geometry_draw(light.shadow_caster, mesh_component.mesh);
					}
				}

				++light_count;
			}
		}
#endif
	}
}