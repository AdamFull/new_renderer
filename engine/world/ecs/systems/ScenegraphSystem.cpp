#include "ScenegraphSystem.h"

#include <entt/entt.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <world/ecs/components/BaseComponent.h>
#include <world/ecs/components/HierarchyComponent.h>
#include <world/ecs/components/TransformComponent.h>

using namespace engine::world::ecs;

CScenegraphSystem::CScenegraphSystem(entt::registry* registry, uint32_t root)
{
	m_pRegistry = registry;
	m_root = root;
}

void CScenegraphSystem::__create()
{

}

void CScenegraphSystem::__update(float fDt)
{
	if (!m_pRegistry->valid(static_cast<entt::entity>(m_root)))
		return;

	buildHierarchy(m_pRegistry, m_root);
}

void CScenegraphSystem::buildHierarchy(entt::registry* registry, uint32_t node)
{
	auto entity = static_cast<entt::entity>(node);
	auto& base = registry->get<FBaseComponent>(entity);
	auto& transform = registry->get<FTransformComponent>(entity);
	auto& hierarchy = registry->get<FHierarchyComponent>(entity);

	// Check is update disabled for branch
	if (!base.enabled)
		return;

	transform.oldmodel = transform.gmodel;
	initMatrix(&transform);

	auto parent = static_cast<entt::entity>(hierarchy.parent);
	if (registry->valid(parent))
	{
		auto& ptransform = registry->get<FTransformComponent>(parent);
		transform.gmodel = ptransform.gmodel * transform.gmodel;
	}

	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(transform.gmodel, transform.gscale, transform.grotation, transform.gposition, skew, perspective);

	for (const auto& child : hierarchy.children)
		buildHierarchy(registry, child);
}

void CScenegraphSystem::initMatrix(FTransformComponent* transform)
{
	transform->gmodel = glm::translate(glm::mat4(1.0f), transform->lposition) * glm::mat4(transform->lrotation) * glm::scale(glm::mat4(1.0f), transform->lscale) * transform->lmodel;
}

void CScenegraphSystem::initMatrices(entt::registry* registry, uint32_t node)
{
}

void CScenegraphSystem::calculateMatrices(entt::registry* registry, uint32_t node)
{
}