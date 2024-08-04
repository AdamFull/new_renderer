#include "LightingSystem.h"

#include <engine/world/EntityManager.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>
#include <engine/world/ecs/components/DirectionalLightComponent.h>
#include <engine/world/ecs/components/PointLightComponent.h>
#include <engine/world/ecs/components/SpotLightComponent.h>

#include <engine/profiler/Profiler.h>

using namespace engine;
using namespace engine::render;
using namespace engine::world::ecs;

CLightingSystem::CLightingSystem(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;

	CEntityManager::RegisterComponent<FDirectionalLightComponent>();
	CEntityManager::RegisterComponent<FPointLightComponent>();
	CEntityManager::RegisterComponent<FSpotLightComponent>();
}

void CLightingSystem::__Create()
{
}

void CLightingSystem::__Update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& render_packet = m_pRenderInterface->GetRenderPacket();
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	auto& world_stats = render_world->GetStats();
	
	// Collecting directional lights
	{
		world_stats.directionalLightCount = 0ull;

		auto view = CEntityManager::View<FTransformComponent, FBaseComponent, FDirectionalLightComponent>();
		for (auto [entity, transform, base, light] : view.each())
		{
			if (!base.enabled)
				continue;

			if (world_stats.directionalLightCount > max_directional_light_count)
				break;

			FLightCommit commit;
			commit.type = light_type_directional;
			commit.direction = glm::normalize(transform.grotation * glm::vec3(0.f, 0.f, -1.f));
			commit.color = light.color;
			commit.intencity = light.intencity;
			commit.shadowIndex = light.shadowIndex;
			commit.castShadows = utl::cast<int>(light.castShadows);

			render_world->UpdateLight(commit);
		}
	}

	// Collecting point lights
	{
		world_stats.pointLightCount = 0ull;

		auto view = CEntityManager::View<FTransformComponent, FBaseComponent, FPointLightComponent>();
		for (auto [entity, transform, base, light] : view.each())
		{
			if (!base.enabled)
				continue;

			if (world_stats.pointLightCount > max_point_light_count)
				break;

			// Calculate light data
			FLightCommit commit;
			commit.type = light_type_point;
			commit.position = transform.gposition;
			commit.color = light.color;
			commit.intencity = light.intencity;
			commit.radius = light.radius;
			commit.shadowIndex = light.shadowIndex;
			commit.castShadows = utl::cast<int>(light.castShadows);

			render_world->UpdateLight(commit);
		}
	}

	// Collecting spot lights
	{
		world_stats.spotLightCount = 0ull;

		// TODO: direction as dot product of rotation and some vec
		auto view = CEntityManager::View<FTransformComponent, FSpotLightComponent>();
		for (auto [entity, transform, light] : view.each())
		{
			if (world_stats.spotLightCount > max_spot_light_count)
				break;

			FLightCommit commit;
			commit.type = light_type_spot;
			commit.position = transform.gposition;
			commit.direction = light.toTarget ? light.target : glm::normalize(transform.grotation * glm::vec3(0.f, 0.f, -1.f));
			commit.color = light.color;
			commit.intencity = light.intencity;
			commit.lightAngleScale = 1.f / (glm::max)(0.001f, glm::cos(light.innerAngle) - glm::cos(light.outerAngle));
			commit.lightAngleOffset = -glm::cos(light.outerAngle) * commit.lightAngleScale;
			commit.toTarget = utl::cast<int>(light.toTarget);
			commit.shadowIndex = light.shadowIndex;
			commit.castShadows = utl::cast<int>(light.castShadows);

			render_world->UpdateLight(commit);
		}
	}
}