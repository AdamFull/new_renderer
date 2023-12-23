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
}

void CCameraSystem::__update(float fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = m_pSceneGraph->getRegistry();
	auto& render_packet = m_pRenderInterface->getRenderPacket();
	auto* render_context = m_pRenderInterface->getRenderContext();

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

			// Update camera projecion
			auto render_resolution = render_context->getRenderResolution();
			if (render_resolution.x != camera.viewportExtent.x || render_resolution.y != camera.viewportExtent.y)
			{
				render_camera.dimensions.x = camera.viewportExtent.x = render_resolution.x;
				render_camera.dimensions.y = camera.viewportExtent.y = render_resolution.y;

				if(camera.type == ECameraType::eOrthographic)
					render_camera.projection = camera.projection = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
				else
				{
					auto aspect = static_cast<float>(render_resolution.x) / static_cast<float>(render_resolution.y);
					render_camera.projection = camera.projection = glm::perspective(camera.fieldOfView, aspect, camera.nearPlane, camera.farPlane);
				}

				render_camera.invProjection = camera.invProjection = glm::inverse(camera.projection);
			}

			if (camera.moved)
			{
				render_camera.view = camera.view = glm::lookAt(transform.gposition, transform.gposition + camera.forward, camera.up);
				render_camera.invView = camera.invView = glm::inverse(camera.view);

				render_packet->camera.viewProjection = render_packet->camera.projection * render_packet->camera.view;
				render_packet->camera.invViewProjection = glm::inverse(render_packet->camera.viewProjection);

				camera.moved = false;
			}

			render_packet->camera.nearp = camera.nearPlane;
			render_packet->camera.farp = camera.farPlane;

			break;
		}
	}
}
