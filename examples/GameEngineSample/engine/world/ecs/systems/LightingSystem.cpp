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
		render_world{ renderer.GetRenderWorld() } {
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

				if (directional_light_count > shlib::max_directional_light_count)
					break;

				shlib::LightCommit commit;
				commit.type = shlib::light_type_directional;
				commit.direction = glm::normalize(transform.grotation * glm::vec3(0.f, 0.f, -1.f));
				commit.color = light.color;
				commit.intencity = light.intencity;
				commit.shadowIndex = light.shadowIndex;
				commit.castShadows = static_cast<int>(light.castShadows);

				render_world.AddLight(commit);
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

				if (point_light_count > shlib::max_point_light_count)
					break;

				// Calculate light data
				shlib::LightCommit commit;
				commit.type = shlib::light_type_point;
				commit.position = transform.gposition;
				commit.color = light.color;
				commit.intencity = light.intencity;
				commit.radius = light.radius;
				commit.shadowIndex = light.shadowIndex;
				commit.castShadows = static_cast<int>(light.castShadows);

				render_world.AddLight(commit);
				point_light_count++;
			}
		}

		// Collecting spot lights
		{
			uint32_t spot_light_count{ 0u };

			// TODO: direction as dot product of rotation and some vec
			auto view = EntityManager::view<TransformComponent, SpotLightComponent>();
			for (auto [entity, transform, light] : view.each()) {
				if (spot_light_count > shlib::max_spot_light_count)
					break;

				shlib::LightCommit commit;
				commit.type = shlib::light_type_spot;
				commit.position = transform.gposition;
				commit.direction = light.toTarget ? light.target : glm::normalize(transform.grotation * glm::vec3(0.f, 0.f, -1.f));
				commit.color = light.color;
				commit.intencity = light.intencity;
				commit.lightAngleScale = 1.f / (glm::max)(0.001f, glm::cos(light.innerAngle) - glm::cos(light.outerAngle));
				commit.lightAngleOffset = -glm::cos(light.outerAngle) * commit.lightAngleScale;
				commit.toTarget = static_cast<int>(light.toTarget);
				commit.shadowIndex = light.shadowIndex;
				commit.castShadows = static_cast<int>(light.castShadows);

				render_world.AddLight(commit);
				spot_light_count++;
			}
		}
	}

	void LightingSystem::on_construct_directional_light(entt::registry& registry, entt::entity entity) {
		auto& light = registry.get<DirectionalLightComponent>(entity);

		for (int32_t i = 0; i < shlib::cascade_shadow_map_cascade_count; ++i) {
			auto [camera_id, render_camera] = render_world.CreateCamera();
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
			auto [camera_id, render_camera] = render_world.CreateCamera();
			light.shadow_casters[i] = camera_id;
		}
	}

	void LightingSystem::on_destruct_pointl_light(entt::registry& registry, entt::entity entity) {
		auto& light = registry.get<PointLightComponent>(entity);

		// TODO: Delete render cameras
	}

	void LightingSystem::on_construct_spot_light(entt::registry& registry, entt::entity entity) {
		auto& light = registry.get<SpotLightComponent>(entity);

		auto [camera_id, render_camera] = render_world.CreateCamera();
		light.shadow_caster = camera_id;
	}

	void LightingSystem::on_destruct_spot_light(entt::registry& registry, entt::entity entity) {
		auto& light = registry.get<SpotLightComponent>(entity);

		// TODO: Delete render cameras
	}
}