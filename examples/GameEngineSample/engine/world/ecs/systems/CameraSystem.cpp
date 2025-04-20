#include "CameraSystem.h"

#include "../../EntityManager.h"
#include "../components/BaseComponent.h"
#include "../components/CameraComponent.h"
#include "../components/TransformComponent.h"

#include <edge/util/utl_profiler.h>

namespace engine::world::ecs
{
	CameraSystem::CameraSystem(render::IRenderer& renderer) :
		render_world{ renderer.get_render_world() } {
		auto& registry = EntityManager::get_registry();

		EntityManager::register_component<CameraComponent>();
		registry.on_construct<CameraComponent>().connect<&CameraSystem::on_construct>(*this);
		registry.on_destroy<CameraComponent>().connect<&CameraSystem::on_destruct>(*this);
	}

	void CameraSystem::__create() {
	}

	void CameraSystem::__update(float fDt) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto view = EntityManager::view<TransformComponent, BaseComponent, CameraComponent>();
		for (auto [entity, transform, base, camera] : view.each()) {
			// TODO: add something like active camera tag
			if (base.enabled) {
				camera.forward = glm::normalize(glm::rotate(transform.grotation, glm::vec3(0.f, 0.f, -1.f)));
				camera.right = glm::normalize(glm::cross(camera.forward, glm::vec3{ 0.f, 1.f, 0.f }));
				camera.up = glm::normalize(glm::cross(camera.right, camera.forward));

				auto& render_camera = render_world.get_render_camera(camera.frustumID);
				render_camera.fov = camera.fieldOfView;
				render_camera.z_near = camera.nearPlane;
				render_camera.z_far = camera.farPlane;

				render_world.update_render_camera(camera.frustumID, transform.gposition, transform.grotation);
				break;
			}
		}
	}

	void CameraSystem::on_construct(entt::registry& registry, entt::entity entity) {
		auto [camera_id, render_camera] = render_world.create_camera();

		auto& camera = registry.get<CameraComponent>(entity);
		camera.frustumID = camera_id;
	}

	void CameraSystem::on_destruct(entt::registry& registry, entt::entity entity) {
		auto& camera = registry.get<CameraComponent>(entity);

		// TODO: destroy render camera
	}
}