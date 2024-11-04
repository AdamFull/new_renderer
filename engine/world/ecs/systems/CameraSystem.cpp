#include "CameraSystem.h"

#include <engine/world/EntityManager.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/CameraComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/profiler/Profiler.h>

using namespace engine::world::ecs;

CCameraSystem::CCameraSystem(render::IRenderer& renderer) :
	render_world{ renderer.GetRenderWorld() }
{
	CEntityManager::RegisterComponent<FCameraComponent>();
}

void CCameraSystem::__Create()
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = CEntityManager::GetRegistry();

	auto view = registry.view<FTransformComponent, FBaseComponent, FCameraComponent>();
	for (auto [entity, transform, base, camera] : view.each())
		camera.frustumID = render_world.CreateRenderCamera();
}

void CCameraSystem::__Update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto view = CEntityManager::View<FTransformComponent, FBaseComponent, FCameraComponent>();
	for (auto [entity, transform, base, camera] : view.each())
	{
		// TODO: add something like active camera tag
		if (base.enabled)
		{
			camera.forward = glm::normalize(glm::rotate(transform.grotation, glm::vec3(0.f, 0.f, -1.f)));
			camera.right = glm::normalize(glm::cross(camera.forward, glm::vec3{ 0.f, 1.f, 0.f }));
			camera.up = glm::normalize(glm::cross(camera.right, camera.forward));

			auto& render_camera = render_world.GetRenderCamera(camera.frustumID);
			render_camera.fov = camera.fieldOfView;
			render_camera.z_near = camera.nearPlane;
			render_camera.z_far = camera.farPlane;

			render_world.UpdateRenderCamera(camera.frustumID, transform.gposition, transform.grotation);
			break;
		}
	}
}
