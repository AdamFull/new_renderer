#include "LightingSystem.h"

#include "../../EntityManager.h"
#include "../components/BaseComponent.h"
#include "../components/TransformComponent.h"
#include "../components/DirectionalLightComponent.h"
#include "../components/PointLightComponent.h"
#include "../components/SpotLightComponent.h"

#include <edge/util/utl_profiler.h>

using namespace engine::render;

namespace engine::world::ecs {
	LightingSystem::LightingSystem(render::IRenderer& renderer) :
		render_world{ renderer.get_render_world() } {
		auto& registry = EntityManager::get_registry();

		// TODO register constructors and destructors here
		EntityManager::register_component<DirectionalLightComponent>();
		registry.on_construct<DirectionalLightComponent>().connect<&LightingSystem::on_construct_directional_light>(*this);
		registry.on_destroy<DirectionalLightComponent>().connect<&LightingSystem::on_destruct_directional_light>(*this);

		EntityManager::register_component<PointLightComponent>();
		registry.on_construct<PointLightComponent>().connect<&LightingSystem::on_construct_pointl_light>(*this);
		registry.on_destroy<PointLightComponent>().connect<&LightingSystem::on_destruct_pointl_light>(*this);

		EntityManager::register_component<SpotLightComponent>();
		registry.on_construct<SpotLightComponent>().connect<&LightingSystem::on_construct_spot_light>(*this);
		registry.on_destroy<SpotLightComponent>().connect<&LightingSystem::on_destruct_spot_light>(*this);
	}

	void LightingSystem::__create() {
	}

	void LightingSystem::__update(float fDt) {
		EDGE_PROFILE_ZONE_SCOPED;
		// TODO: RENDER WORLD

		// Collecting directional lights
		{
			uint32_t directional_light_count{ 0u };

			auto view = EntityManager::view<TransformComponent, BaseComponent, DirectionalLightComponent>();
			for (auto [entity, transform, base, light] : view.each()) {
				if (!base.enabled)
					continue;

				if (directional_light_count > constants::max_directional_light_count)
					break;

				interop::LightCommit commit;
				commit.type = interop::LightType::eDirectional;
				commit.direction = glm::normalize(transform.grotation * glm::vec3(0.f, 0.f, -1.f));
				commit.color = light.color;
				commit.intencity = light.intencity;
				commit.shadow_index = light.shadowIndex;
				commit.cast_shadows = static_cast<int>(light.castShadows);

				render_world.add_light(commit);
				directional_light_count++;
			}
		}

		// Collecting point lights
		{
			uint32_t point_light_count{ 0u };

			auto view = EntityManager::view<TransformComponent, BaseComponent, PointLightComponent>();
			for (auto [entity, transform, base, light] : view.each()) {
				if (!base.enabled)
					continue;

				if (point_light_count > constants::max_point_light_count)
					break;

				// Calculate light data
				interop::LightCommit commit;
				commit.type = interop::LightType::ePoint;
				commit.position = transform.gposition;
				commit.color = light.color;
				commit.intencity = light.intencity;
				commit.radius = light.radius;
				commit.shadow_index = light.shadowIndex;
				commit.cast_shadows = static_cast<int>(light.castShadows);

				render_world.add_light(commit);
				point_light_count++;
			}
		}

		// Collecting spot lights
		{
			uint32_t spot_light_count{ 0u };

			// TODO: direction as dot product of rotation and some vec
			auto view = EntityManager::view<TransformComponent, SpotLightComponent>();
			for (auto [entity, transform, light] : view.each()) {
				if (spot_light_count > constants::max_spot_light_count)
					break;

				interop::LightCommit commit;
				commit.type = interop::LightType::eSpot;
				commit.position = transform.gposition;
				commit.direction = light.toTarget ? light.target : glm::normalize(transform.grotation * glm::vec3(0.f, 0.f, -1.f));
				commit.color = light.color;
				commit.intencity = light.intencity;
				commit.light_angle_scale = 1.f / (glm::max)(0.001f, glm::cos(light.innerAngle) - glm::cos(light.outerAngle));
				commit.light_angle_offset = -glm::cos(light.outerAngle) * commit.light_angle_scale;
				commit.to_target = static_cast<int>(light.toTarget);
				commit.shadow_index = light.shadowIndex;
				commit.cast_shadows = static_cast<int>(light.castShadows);

				render_world.add_light(commit);
				spot_light_count++;
			}
		}
	}

	void LightingSystem::on_construct_directional_light(entt::registry& registry, entt::entity entity) {
		auto& light = registry.get<DirectionalLightComponent>(entity);

		for (int32_t i = 0; i < constants::cascade_shadow_map_cascade_count; ++i) {
			auto [camera_id, render_camera] = render_world.create_camera();
			light.shadow_casters.emplace_back(camera_id);
		}
	}

	void LightingSystem::on_destruct_directional_light(entt::registry& registry, entt::entity entity) {
		auto& light = registry.get<DirectionalLightComponent>(entity);

		// TODO: Delete render cameras
	}

	void LightingSystem::on_construct_pointl_light(entt::registry& registry, entt::entity entity) {
		auto& light = registry.get<PointLightComponent>(entity);

		for (int32_t i = 0; i < light.shadow_casters.size(); ++i) {
			auto [camera_id, render_camera] = render_world.create_camera();
			light.shadow_casters[i] = camera_id;
		}
	}

	void LightingSystem::on_destruct_pointl_light(entt::registry& registry, entt::entity entity) {
		auto& light = registry.get<PointLightComponent>(entity);

		// TODO: Delete render cameras
	}

	void LightingSystem::on_construct_spot_light(entt::registry& registry, entt::entity entity) {
		auto& light = registry.get<SpotLightComponent>(entity);

		auto [camera_id, render_camera] = render_world.create_camera();
		light.shadow_caster = camera_id;
	}

	void LightingSystem::on_destruct_spot_light(entt::registry& registry, entt::entity entity) {
		auto& light = registry.get<SpotLightComponent>(entity);

		// TODO: Delete render cameras
	}
}