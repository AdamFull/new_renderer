#include "LightingSystem.h"

#include "../../EntityManager.h"
#include "../components/BaseComponent.h"
#include "../components/TransformComponent.h"
#include "../components/DirectionalLightComponent.h"
#include "../components/PointLightComponent.h"
#include "../components/SpotLightComponent.h"

#include <edge/util/Profiler.h>

using namespace engine::render;

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			LightingSystem::LightingSystem(render::IRenderer& renderer) :
				render_world{ renderer.GetRenderWorld() }
			{
				auto& registry = EntityManager::GetRegistry();

				// TODO register constructors and destructors here
				EntityManager::RegisterComponent<FDirectionalLightComponent>();
				registry.on_construct<FDirectionalLightComponent>().connect<&LightingSystem::OnConstructDirectionalLight>(*this);
				registry.on_destroy<FDirectionalLightComponent>().connect<&LightingSystem::OnDestructDirectionalLight>(*this);

				EntityManager::RegisterComponent<FPointLightComponent>();
				registry.on_construct<FPointLightComponent>().connect<&LightingSystem::OnConstructPointlLight>(*this);
				registry.on_destroy<FPointLightComponent>().connect<&LightingSystem::OnDestructPointlLight>(*this);

				EntityManager::RegisterComponent<FSpotLightComponent>();
				registry.on_construct<FSpotLightComponent>().connect<&LightingSystem::OnConstructSpotLight>(*this);
				registry.on_destroy<FSpotLightComponent>().connect<&LightingSystem::OnDestructSpotLight>(*this);
			}

			void LightingSystem::__Create()
			{
			}

			void LightingSystem::__Update(f32 fDt)
			{
				EDGE_PROFILE_ZONE_SCOPED;
				// TODO: RENDER WORLD

				// Collecting directional lights
				{
					uint32_t directional_light_count{ 0u };

					auto view = EntityManager::View<FTransformComponent, FBaseComponent, FDirectionalLightComponent>();
					for (auto [entity, transform, base, light] : view.each())
					{
						if (!base.enabled)
							continue;

						if (directional_light_count > shlib::max_directional_light_count)
							break;

						shlib::LightCommit commit;
						commit.type = shlib::light_type_directional;
						commit.direction = glm::normalize(transform.grotation * glm::vec3(0.f, 0.f, -1.f));
						commit.color = light.color;
						commit.intencity = light.intencity;
						commit.shadowIndex = light.shadowIndex;
						commit.castShadows = utl::cast<int>(light.castShadows);

						render_world.AddLight(commit);
						directional_light_count++;
					}
				}

				// Collecting point lights
				{
					uint32_t point_light_count{ 0u };

					auto view = EntityManager::View<FTransformComponent, FBaseComponent, FPointLightComponent>();
					for (auto [entity, transform, base, light] : view.each())
					{
						if (!base.enabled)
							continue;

						if (point_light_count > shlib::max_point_light_count)
							break;

						// Calculate light data
						shlib::LightCommit commit;
						commit.type = shlib::light_type_point;
						commit.position = transform.gposition;
						commit.color = light.color;
						commit.intencity = light.intencity;
						commit.radius = light.radius;
						commit.shadowIndex = light.shadowIndex;
						commit.castShadows = utl::cast<int>(light.castShadows);

						render_world.AddLight(commit);
						point_light_count++;
					}
				}

				// Collecting spot lights
				{
					uint32_t spot_light_count{ 0u };

					// TODO: direction as dot product of rotation and some vec
					auto view = EntityManager::View<FTransformComponent, FSpotLightComponent>();
					for (auto [entity, transform, light] : view.each())
					{
						if (spot_light_count > shlib::max_spot_light_count)
							break;

						shlib::LightCommit commit;
						commit.type = shlib::light_type_spot;
						commit.position = transform.gposition;
						commit.direction = light.toTarget ? light.target : glm::normalize(transform.grotation * glm::vec3(0.f, 0.f, -1.f));
						commit.color = light.color;
						commit.intencity = light.intencity;
						commit.lightAngleScale = 1.f / (glm::max)(0.001f, glm::cos(light.innerAngle) - glm::cos(light.outerAngle));
						commit.lightAngleOffset = -glm::cos(light.outerAngle) * commit.lightAngleScale;
						commit.toTarget = utl::cast<int>(light.toTarget);
						commit.shadowIndex = light.shadowIndex;
						commit.castShadows = utl::cast<int>(light.castShadows);

						render_world.AddLight(commit);
						spot_light_count++;
					}
				}
			}

			void LightingSystem::OnConstructDirectionalLight(entt::registry& registry, entt::entity entity)
			{
				auto& light = registry.get<FDirectionalLightComponent>(entity);

				for (int32_t i = 0; i < shlib::cascade_shadow_map_cascade_count; ++i)
				{
					light.shadow_casters.emplace_back(render_world.CreateRenderCamera());
				}
			}

			void LightingSystem::OnDestructDirectionalLight(entt::registry& registry, entt::entity entity)
			{
				auto& light = registry.get<FDirectionalLightComponent>(entity);

				// TODO: Delete render cameras
			}

			void LightingSystem::OnConstructPointlLight(entt::registry& registry, entt::entity entity)
			{
				auto& light = registry.get<FPointLightComponent>(entity);

				for (int32_t i = 0; i < light.shadow_casters.size(); ++i)
				{
					light.shadow_casters[i] = render_world.CreateRenderCamera();
				}
			}

			void LightingSystem::OnDestructPointlLight(entt::registry& registry, entt::entity entity)
			{
				auto& light = registry.get<FPointLightComponent>(entity);

				// TODO: Delete render cameras
			}

			void LightingSystem::OnConstructSpotLight(entt::registry& registry, entt::entity entity)
			{
				auto& light = registry.get<FSpotLightComponent>(entity);

				light.shadow_caster = render_world.CreateRenderCamera();
			}

			void LightingSystem::OnDestructSpotLight(entt::registry& registry, entt::entity entity)
			{
				auto& light = registry.get<FSpotLightComponent>(entity);

				// TODO: Delete render cameras
			}
		}
	}
}