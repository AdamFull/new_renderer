#include "ScenegraphSystem.h"

#include <glm/gtx/matrix_decompose.hpp>

#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/HierarchyComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>
#include <engine/world/EntityManager.h>

#include <engine/profiler/Profiler.h>

using namespace engine::world::ecs;

CScenegraphSystem::CScenegraphSystem()
{
	EntityManager::RegisterComponent<FRootComponent>(EComponentFlagBits::eHiddenInEditor);
	EntityManager::RegisterComponent<FBaseComponent>(EComponentFlagBits::eHiddenInEditor);
	EntityManager::RegisterComponent<FHierarchyComponent>(EComponentFlagBits::eHiddenInEditor);
	EntityManager::RegisterComponent<FTransformComponent>();
}

void CScenegraphSystem::__Create()
{
	TRACY_PROFILE_FUNCTION();
}

void CScenegraphSystem::__Update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = EntityManager::GetRegistry();
	auto root = EntityManager::GetRoot();
	if (!EntityManager::IsValid(root))
		return;

	buildHierarchy(root);
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