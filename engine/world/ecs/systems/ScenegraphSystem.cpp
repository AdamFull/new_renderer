#include "ScenegraphSystem.h"

#include <glm/gtx/matrix_decompose.hpp>

#include "../components/BaseComponent.h"
#include "../components/HierarchyComponent.h"
#include "../components/TransformComponent.h"
#include "../../EntityManager.h"

#include <edge/util/Profiler.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			CScenegraphSystem::CScenegraphSystem(render::IRenderer& renderer) :
				render_world{ renderer.GetRenderWorld() }
			{
				EntityManager::RegisterComponent<FRootComponent>(EComponentFlagBits::eHiddenInEditor);
				EntityManager::RegisterComponent<FBaseComponent>(EComponentFlagBits::eHiddenInEditor);
				EntityManager::RegisterComponent<FHierarchyComponent>(EComponentFlagBits::eHiddenInEditor);
				EntityManager::RegisterComponent<FTransformComponent>();

				auto& registry = EntityManager::GetRegistry();

				registry.on_construct<FTransformComponent>().connect<&CScenegraphSystem::OnConstruct>(*this);
				registry.on_destroy<FTransformComponent>().connect<&CScenegraphSystem::OnDestruct>(*this);
			}

			void CScenegraphSystem::__Create()
			{
			}

			void CScenegraphSystem::__Update(f32 fDt)
			{
				EDGE_PROFILE_ZONE_SCOPED;

				auto& registry = EntityManager::GetRegistry();
				auto root = EntityManager::GetRoot();
				if (!EntityManager::IsValid(root))
					return;

				buildHierarchy(root);

				auto view = EntityManager::View<FTransformComponent, FBaseComponent>();
				for (auto [entity, transform, base] : view.each())
				{
					if (!base.visible)
						continue;

					render_world.UpdateNode(transform.node_index, transform.gmodel);
				}
			}

			void CScenegraphSystem::OnConstruct(entt::registry& registry, entt::entity entity)
			{
				auto& transform = registry.get<FTransformComponent>(entity);

				transform.node_index = render_world.CreateNode();
			}

			void CScenegraphSystem::OnDestruct(entt::registry& registry, entt::entity entity)
			{
				auto& transform = registry.get<FTransformComponent>(entity);

				// TODO: free node
			}

			void CScenegraphSystem::buildHierarchy(entt::entity node)
			{
				auto& base = EntityManager::GetComponent<FBaseComponent>(node);
				auto& transform = EntityManager::GetComponent<FTransformComponent>(node);
				auto& hierarchy = EntityManager::GetComponent<FHierarchyComponent>(node);

				// Check is update disabled for branch
				if (!base.enabled)
					return;

				transform.oldmodel = transform.gmodel;
				initMatrix(&transform);

				auto parent = hierarchy.parent;
				if (EntityManager::IsValid(parent))
				{
					auto& ptransform = EntityManager::GetComponent<FTransformComponent>(parent);
					transform.gmodel = ptransform.gmodel * transform.gmodel;
				}

				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(transform.gmodel, transform.gscale, transform.grotation, transform.gposition, skew, perspective);

				transform.grotation = glm::normalize(transform.grotation);

				for (const auto& child : hierarchy.children)
					buildHierarchy(child);
			}

			void CScenegraphSystem::initMatrix(FTransformComponent* transform)
			{
				transform->gmodel = glm::translate(glm::mat4(1.0f), transform->lposition) * glm::mat4(transform->lrotation) * glm::scale(glm::mat4(1.0f), transform->lscale) * transform->lmodel;
			}

			void CScenegraphSystem::initMatrices(entt::registry* registry, entt::entity node)
			{
			}

			void CScenegraphSystem::calculateMatrices(entt::registry* registry, entt::entity node)
			{
			}

		}
	}
}