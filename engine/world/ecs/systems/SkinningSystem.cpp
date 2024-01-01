#include "SkinningSystem.h"

#include <engine/world/SceneGraph.h>

#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/HierarchyComponent.h>
#include <engine/world/ecs/components/MeshComponent.h>
#include <engine/world/ecs/components/RootComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/profiler/Profiler.h>

using namespace engine::world::ecs;

void debug_draw_skeleton(entt::registry* registry, entt::entity node, glm::mat4 parent_matrix = glm::mat4(1.f))
{
	if (!registry->valid(node))
		return;

	auto& node_transform = registry->get<FTransformComponent>(node);
	auto& node_hierarchy = registry->get<FHierarchyComponent>(node);

	glm::vec3 node_position = glm::vec3(node_transform.gmodel[3]);

	if (parent_matrix != glm::mat4(1.0f))
	{
		glm::vec3 parent_position = glm::vec3(parent_matrix[3]);
		// TODO: think about what to do with debug drawing
		//debug_draw->drawDebugLine(parent_position, node_position);
	}

	for (auto child : node_hierarchy.children)
		debug_draw_skeleton(registry, child, node_transform.gmodel);
}

CSkinningSystem::CSkinningSystem(CSceneGraph* scenegraph, render::IRenderInterface* render_interface)
{
	m_pSceneGraph = scenegraph;
	m_pRenderInterface = render_interface;
}

void CSkinningSystem::__create()
{
	TRACY_PROFILE_FUNCTION();
}

void CSkinningSystem::__update(float fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = m_pSceneGraph->getRegistry();
	auto root = m_pSceneGraph->getRoot();
	auto& root_component = m_pSceneGraph->getComponent<FRootComponent>(root);

	auto view = registry->view<FTransformComponent, FBaseComponent, FMeshComponent>();
	for (auto [entity, transform, base, mesh] : view.each())
	{
		if (!base.enabled || mesh.skin < 0)
			continue;

		auto& skin = root_component.skins[mesh.skin];

#ifndef BUILD_WITHOUT_DEBUGGIND
		debug_draw_skeleton(registry.get(), skin.root);
#endif

		// Calculate joint matrices
		auto inv_transform = glm::inverse(transform.gmodel);
		for (uint32_t i = 0; i < skin.joints.size(); i++)
		{
			auto& jtransform = m_pSceneGraph->getComponent<FTransformComponent>(skin.joints[i]);
			skin.jointMatrices[i] = jtransform.gmodel * skin.inverseBindMatrices[i];
			skin.jointMatrices[i] = inv_transform * skin.jointMatrices[i];
		}
	}

	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& world_stats = render_world->getStats();
	world_stats.skinCount = 0ull;

	for (size_t idx = 0ull; idx < root_component.skins.size(); ++idx)
	{
		auto& skin = root_component.skins[idx];
		render_world->updateSkin(skin.jointMatrices);
	}
}