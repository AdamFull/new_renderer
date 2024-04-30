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
		camera.frustumID = render_world->registerFrustum(ERenderGeometryCullerType::eScene);
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
			auto& render_camera = render_packet->camera;

			render_camera.forward = camera.forward = glm::normalize(glm::rotate(transform.grotation, glm::vec3(0.f, 0.f, -1.f)));
			render_camera.right = camera.right = glm::normalize(glm::cross(camera.forward, glm::vec3{ 0.f, 1.f, 0.f }));
			render_camera.up = camera.up = glm::normalize(glm::cross(camera.right, camera.forward));

			if (camera.moved)
			{
				render_camera.view = camera.view = glm::lookAt(transform.gposition, transform.gposition + camera.forward, camera.up);
				render_camera.invView = camera.invView = glm::inverse(camera.view);
				render_packet->camera.wasUpdated = true;

				camera.moved = false;
			}

			render_packet->camera.nearp = camera.nearPlane;
			render_packet->camera.farp = camera.farPlane;

			//render_world->updateFrustum(camera.frustumID, camera.view, camera.projection);
			break;
		}
	}
}
