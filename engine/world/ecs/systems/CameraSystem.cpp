#include "CameraSystem.h"

#include <engine/world/SceneGraph.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/CameraComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/profiler/Profiler.h>

using namespace engine::render;
using namespace engine::world::ecs;

CCameraSystem::CCameraSystem(CSceneGraph* scenegraph, IRenderInterface* render_interface)
{
	m_pSceneGraph = scenegraph;
	m_pRenderInterface = render_interface;
}

void CCameraSystem::__create()
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = m_pSceneGraph->getRegistry();
	auto& render_world = m_pRenderInterface->getRenderWorld();

	auto view = registry->view<FTransformComponent, FBaseComponent, FCameraComponent>();
	for (auto [entity, transform, base, camera] : view.each())
		camera.frustumID = render_world->registerCamera(ERenderGeometryCullerType::eScene);
}

void CCameraSystem::__update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = m_pSceneGraph->getRegistry();
	auto& render_packet = m_pRenderInterface->getRenderPacket();
	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_world = m_pRenderInterface->getRenderWorld();

	auto view = registry->view<FTransformComponent, FBaseComponent, FCameraComponent>();
	for (auto [entity, transform, base, camera] : view.each())
	{
		// TODO: add something like active camera tag
		if (base.enabled)
		{
			camera.forward = glm::normalize(glm::rotate(transform.grotation, glm::vec3(0.f, 0.f, -1.f)));
			camera.right = glm::normalize(glm::cross(camera.forward, glm::vec3{ 0.f, 1.f, 0.f }));
			camera.up = glm::normalize(glm::cross(camera.right, camera.forward));

			if (camera.moved)
			{
				glm::mat4 prev_view = camera.view;
				glm::mat4 prev_projection = camera.projection;
				camera.view = glm::lookAt(transform.gposition, transform.gposition + camera.forward, camera.up);
				camera.projection = glm::perspective(camera.fieldOfView, aspect, camera.nearPlane, camera.farPlane);

				if(camera.active)
					render_packet->camera.wasUpdated = true;

				render_world->updateCamera(camera.frustumID, camera.view, viewport.projection);

				camera.moved = false;
			}

			render_packet->camera.nearp = camera.nearPlane;
			render_packet->camera.farp = camera.farPlane;

			//glm::perspective(render_packet->camera.fov, aspect, render_packet->camera.nearp, render_packet->camera.farp);
			break;
		}
	}
}
