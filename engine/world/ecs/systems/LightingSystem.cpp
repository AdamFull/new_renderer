#include "LightingSystem.h"

#include <engine/world/SceneGraph.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>
#include <engine/world/ecs/components/DirectionalLightComponent.h>
#include <engine/world/ecs/components/PointLightComponent.h>
#include <engine/world/ecs/components/SpotLightComponent.h>

#include <engine/profiler/Profiler.h>

using namespace engine;
using namespace engine::render;
using namespace engine::world::ecs;

CLightingSystem::CLightingSystem(CSceneGraph* scenegraph, IRenderInterface* render_interface)
{
	m_pSceneGraph = scenegraph;
	m_pRenderInterface = render_interface;
}

void CLightingSystem::__create()
{
	TRACY_PROFILE_FUNCTION();
}

void CLightingSystem::__update(float fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = m_pSceneGraph->getRegistry();
	auto& render_packet = m_pRenderInterface->getRenderPacket();
	
	// Collecting directional lights
	{
		auto& lights = render_packet->dlights;
		lights.lcount &= 0u;

		auto view = registry->view<FTransformComponent, FBaseComponent, FDirectionalLightComponent>();
		for (auto [entity, transform, base, light] : view.each())
		{
			if (!base.enabled)
				continue;

			if (lights.lcount > max_directional_light_count)
				break;

			FDirectionalLightCommit commit;
			commit.direction = glm::normalize(transform.grotation * glm::vec3(0.f, 0.f, -1.f));
			commit.color = light.color;
			commit.intencity = light.intencity;
			commit.shadowIndex = light.shadowIndex;
			commit.castShadows = static_cast<int>(light.castShadows);

			lights.lights->insert(&commit, lights.lcount++);
		}
	}

	// Collecting point lights
	{
		auto& lights = render_packet->plights;
		lights.lcount &= 0u;

		auto view = registry->view<FTransformComponent, FBaseComponent, FPointLightComponent>();
		for (auto [entity, transform, base, light] : view.each())
		{
			if (!base.enabled)
				continue;

			if (lights.lcount > max_point_light_count)
				break;

			// Calculate light data
			FPointLightCommit commit;
			commit.position = transform.gposition;
			commit.color = light.color;
			commit.intencity = light.intencity;
			commit.radius = light.radius;
			commit.shadowIndex = light.shadowIndex;
			commit.castShadows = static_cast<int>(light.castShadows);

			lights.lights->insert(&commit, lights.lcount++);
		}
	}

	// Collecting spot lights
	{
		auto& lights = render_packet->slights;
		lights.lcount &= 0u;

		// TODO: direction as dot product of rotation and some vec
		auto view = registry->view<FTransformComponent, FSpotLightComponent>();
		for (auto [entity, transform, light] : view.each())
		{
			if (lights.lcount > max_spot_light_count)
				break;

			FSpotLightCommit commit;
			commit.position = transform.gposition;
			commit.direction = light.toTarget ? light.target : glm::normalize(transform.grotation * glm::vec3(0.f, 0.f, -1.f));
			commit.color = light.color;
			commit.intencity = light.intencity;
			commit.lightAngleScale = 1.f / (glm::max)(0.001f, glm::cos(light.innerAngle) - glm::cos(light.outerAngle));
			commit.lightAngleOffset = -glm::cos(light.outerAngle) * commit.lightAngleScale;
			commit.toTarget = static_cast<int>(light.toTarget);
			commit.shadowIndex = light.shadowIndex;
			commit.castShadows = static_cast<int>(light.castShadows);

			lights.lights->insert(&commit, lights.lcount++);
		}
	}
}