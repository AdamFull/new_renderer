#include "CameraSystem.h"

#include <engine/world/EntityManager.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/CameraComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/profiler/Profiler.h>

using namespace engine::render;
using namespace engine::world::ecs;

CCameraSystem::CCameraSystem(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CCameraSystem::__Create()
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = CEntityManager::GetRegistry();
	auto& render_world = m_pRenderInterface->GetRenderWorld();

	auto view = registry.view<FTransformComponent, FBaseComponent, FCameraComponent>();
	for (auto [entity, transform, base, camera] : view.each())
		camera.frustumID = render_world->RegisterCamera(ERenderGeometryCullerType::eScene);
}

void CCameraSystem::__Update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& render_world = m_pRenderInterface->GetRenderWorld();

	auto view = CEntityManager::View<FTransformComponent, FBaseComponent, FCameraComponent>();
	for (auto [entity, transform, base, camera] : view.each())
	{
		// TODO: add something like active camera tag
		if (base.enabled)
		{
			camera.forward = glm::normalize(glm::rotate(transform.grotation, glm::vec3(0.f, 0.f, -1.f)));
			camera.right = glm::normalize(glm::cross(camera.forward, glm::vec3{ 0.f, 1.f, 0.f }));
			camera.up = glm::normalize(glm::cross(camera.right, camera.forward));

			//if (camera.moved)
			{
				auto screen_size = render_context->GetScreenRenderRect();
				glm::mat4 prev_view = camera.view;
				glm::mat4 prev_projection = camera.projection;
				camera.view = glm::lookAt(transform.gposition, transform.gposition + camera.forward, camera.up);
				camera.projection = glm::perspective(camera.fieldOfView, screen_size.size.x / screen_size.size.y, camera.nearPlane, camera.farPlane);

				render_world->UpdateCamera(camera.frustumID, camera.view, prev_view, camera.projection, prev_projection, glm::vec2(screen_size.size.x, screen_size.size.y), camera.nearPlane, camera.farPlane);

				camera.moved = false;
			}
			break;
		}
	}
}
