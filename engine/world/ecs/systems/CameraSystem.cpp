#include "CameraSystem.h"

#include <engine/world/EntityManager.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/CameraComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/profiler/Profiler.h>

using namespace engine::world::ecs;

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			CameraSystem::CameraSystem(render::IRenderer& renderer) :
				render_world{ renderer.GetRenderWorld() }
			{
				auto& registry = EntityManager::GetRegistry();

				EntityManager::RegisterComponent<FCameraComponent>();
				registry.on_construct<FCameraComponent>().connect<&CameraSystem::OnConstruct>(*this);
				registry.on_destroy<FCameraComponent>().connect<&CameraSystem::OnDestruct>(*this);
			}

			void CameraSystem::__Create()
			{
			}

			void CameraSystem::__Update(f32 fDt)
			{
				TRACY_PROFILE_FUNCTION();

				auto view = EntityManager::View<FTransformComponent, FBaseComponent, FCameraComponent>();
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

			void CameraSystem::OnConstruct(entt::registry& registry, entt::entity entity)
			{
				auto& camera = registry.get<FCameraComponent>(entity);

				camera.frustumID = render_world.CreateRenderCamera();
			}

			void CameraSystem::OnDestruct(entt::registry& registry, entt::entity entity)
			{
				auto& camera = registry.get<FCameraComponent>(entity);

				// TODO: destroy render camera
			}
		}
	}
}