#include "ScenegraphSystem.h"

#include <glm/gtx/matrix_decompose.hpp>

#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/HierarchyComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>
#include <engine/world/SceneGraph.h>

#include <engine/profiler/Profiler.h>

using namespace engine::world::ecs;

CScenegraphSystem::CScenegraphSystem(CSceneGraph* scenegraph)
{
	m_pSceneGraph = scenegraph;
}

void CScenegraphSystem::__create()
{
	TRACY_PROFILE_FUNCTION();
}

void CScenegraphSystem::__update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = m_pSceneGraph->getRegistry();
	auto root = m_pSceneGraph->getRoot();
	if (!registry->valid(root))
		return;

	buildHierarchy(registry.get(), root);
}

void CScenegraphSystem::buildHierarchy(entt::registry* registry, entt::entity node)
{
	auto& base = registry->get<FBaseComponent>(node);
	auto& transform = registry->get<FTransformComponent>(node);
	auto& hierarchy = registry->get<FHierarchyComponent>(node);

	// Check is update disabled for branch
	if (!base.enabled)
		return;

	transform.oldmodel = transform.gmodel;
	initMatrix(&transform);

	auto parent = hierarchy.parent;
	if (registry->valid(parent))
	{
		auto& ptransform = registry->get<FTransformComponent>(parent);
		transform.gmodel = ptransform.gmodel * transform.gmodel;
	}

	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(transform.gmodel, transform.gscale, transform.grotation, transform.gposition, skew, perspective);

	transform.grotation = glm::normalize(transform.grotation);

	for (const auto& child : hierarchy.children)
		buildHierarchy(registry, child);
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