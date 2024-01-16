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

	auto& registry = m_pSceneGraph->getRegistry();
	auto& render_world = m_pRenderInterface->getRenderWorld();

	{
		size_t shadow_count{ 0ull };
		auto view = registry->view<FTransformComponent, FBaseComponent, FDirectionalLightComponent>();
		for (auto [entity, transform, base, light] : view.each())
		{
			if (light.castShadows && shadow_count < max_directional_light_shadow_count)
			{
				light.frustumIDs.resize(cascade_shadow_map_cascade_count);
				for(size_t idx = 0ull; idx < cascade_shadow_map_cascade_count; ++idx)
					light.frustumIDs[idx] = render_world->registerFrustum(ERenderGeometryCullerType::eCascadedShadow);

				shadow_count++;
			}
		}
	}

	{
		size_t shadow_count{ 0ull };
		auto view = registry->view<FTransformComponent, FBaseComponent, FPointLightComponent>();
		for (auto [entity, transform, base, light] : view.each())
		{
			if (light.castShadows && shadow_count < max_point_light_shadow_count)
			{
				light.frustumID = render_world->registerFrustum(ERenderGeometryCullerType::eOmniShadow);
				shadow_count++;
			}
		}
	}

	{
		size_t shadow_count{ 0ull };
		auto view = registry->view<FTransformComponent, FBaseComponent, FSpotLightComponent>();
		for (auto [entity, transform, base, light] : view.each())
		{
			if (light.castShadows && shadow_count < max_spot_light_shadow_count)
			{
				light.frustumID = render_world->registerFrustum(ERenderGeometryCullerType::eSpotShadow);
				shadow_count++;
			}
		}
	}
}

void CLightingSystem::__update(float fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = m_pSceneGraph->getRegistry();
	auto& render_packet = m_pRenderInterface->getRenderPacket();
	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& world_stats = render_world->getStats();
	
	// Collecting directional lights
	{
		world_stats.directionalLightCount = 0ull;

		auto view = registry->view<FTransformComponent, FBaseComponent, FDirectionalLightComponent>();
		for (auto [entity, transform, base, light] : view.each())
		{
			if (!base.enabled)
				continue;

			if (world_stats.directionalLightCount > max_directional_light_count)
				break;

			FDirectionalLightCommit commit;
			commit.direction = glm::normalize(transform.grotation * glm::vec3(0.f, 0.f, -1.f));
			commit.color = light.color;
			commit.intencity = light.intencity;
			commit.shadowIndex = light.shadowIndex;
			commit.castShadows = static_cast<int>(light.castShadows);

			render_world->addLight(commit);
		}
	}

	// Collecting point lights
	{
		world_stats.pointLightCount = 0ull;

		auto view = registry->view<FTransformComponent, FBaseComponent, FPointLightComponent>();
		for (auto [entity, transform, base, light] : view.each())
		{
			if (!base.enabled)
				continue;

			if (world_stats.pointLightCount > max_point_light_count)
				break;

			// Calculate light data
			FPointLightCommit commit;
			commit.position = transform.gposition;
			commit.color = light.color;
			commit.intencity = light.intencity;
			commit.radius = light.radius;
			commit.shadowIndex = light.shadowIndex;
			commit.castShadows = static_cast<int>(light.castShadows);

			render_world->addLight(commit);
		}
	}

	// Collecting spot lights
	{
		world_stats.spotLightCount = 0ull;

		// TODO: direction as dot product of rotation and some vec
		auto view = registry->view<FTransformComponent, FSpotLightComponent>();
		for (auto [entity, transform, light] : view.each())
		{
			if (world_stats.spotLightCount > max_spot_light_count)
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

			render_world->addLight(commit);
		}
	}
}