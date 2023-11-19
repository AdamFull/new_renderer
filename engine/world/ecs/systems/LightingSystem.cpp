#include "LightingSystem.h"

#include <world/SceneGraph.h>

#include <world/ecs/components/BaseComponent.h>
#include <world/ecs/components/TransformComponent.h>
#include <world/ecs/components/DirectionalLightComponent.h>
#include <world/ecs/components/PointLightComponent.h>
#include <world/ecs/components/SpotLightComponent.h>

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

}

void CLightingSystem::__update(float fDt)
{
	auto& registry = m_pSceneGraph->getRegistry();
	auto& render_packet = m_pRenderInterface->getRenderPacket();
	
	// Collecting directional lights
	{
		auto& directional = render_packet->directionalLights.lights;
		directional.count &= 0u;

		auto view = registry->view<FTransformComponent, FBaseComponent, FDirectionalLightComponent>();
		for (auto [entity, transform, base, light] : view.each())
		{
			if (!base.enabled)
				continue;

			if (directional.count > max_directional_light_count)
				break;

			FDirectionalLightCommit commit;
			commit.direction = glm::normalize(glm::toQuat(transform.gmodel) * glm::vec3(0.f, 0.f, 1.f));
			commit.color = light.color;
			commit.intencity = light.intencity;
			commit.shadowIndex = light.shadowIndex;
			commit.castShadows = static_cast<int>(light.castShadows);

			directional.lights[directional.count++] = commit;
		}
	}

	// Collecting point lights
	{
		auto& point = render_packet->pointLights.lights;
		point.count &= 0u;

		auto view = registry->view<FTransformComponent, FBaseComponent, FPointLightComponent>();
		for (auto [entity, transform, base, light] : view.each())
		{
			if (!base.enabled)
				continue;

			if (point.count > max_point_light_count)
				break;

			// Calculate light data
			FPointLightCommit commit;
			commit.position = transform.gposition;
			commit.color = light.color;
			commit.intencity = light.intencity;
			commit.radius = light.radius;
			commit.shadowIndex = light.shadowIndex;
			commit.castShadows = static_cast<int>(light.castShadows);

			point.lights[point.count++] = commit;
		}
	}

	// Collecting spot lights
	{
		auto& spot = render_packet->spotLights.lights;
		spot.count &= 0u;

		// TODO: direction as dot product of rotation and some vec
		auto view = registry->view<FTransformComponent, FSpotLightComponent>();
		for (auto [entity, transform, light] : view.each())
		{
			if (spot.count > max_spot_light_count)
				break;

			FSpotLightCommit commit;
			commit.position = transform.gposition;
			commit.direction = light.toTarget ? light.target : glm::normalize(glm::toQuat(transform.gmodel) * glm::vec3(0.f, 0.f, 1.f));
			commit.color = light.color;
			commit.intencity = light.intencity;
			commit.lightAngleScale = 1.f / (glm::max)(0.001f, glm::cos(light.innerAngle) - glm::cos(light.outerAngle));
			commit.lightAngleOffset = -glm::cos(light.outerAngle) * commit.lightAngleScale;
			commit.toTarget = static_cast<int>(light.toTarget);
			commit.shadowIndex = light.shadowIndex;
			commit.castShadows = static_cast<int>(light.castShadows);

			spot.lights[spot.count++] = commit;
		}
	}
}