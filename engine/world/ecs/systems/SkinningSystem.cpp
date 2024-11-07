#include "SkinningSystem.h"

#include <engine/world/EntityManager.h>

#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/HierarchyComponent.h>
#include <engine/world/ecs/components/MeshComponent.h>
#include <engine/world/ecs/components/RootComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/profiler/Profiler.h>

#include <execution>

using namespace engine::world;
using namespace engine::world::ecs;

//#define USE_STD_MULTITHREADING

void debug_draw_skeleton(entt::entity node, glm::mat4 parent_matrix = glm::mat4(1.f))
{
	if (!EntityManager::IsValid(node))
		return;

	auto& node_transform = EntityManager::GetComponent<FTransformComponent>(node);
	auto& node_hierarchy = EntityManager::GetComponent<FHierarchyComponent>(node);

	glm::vec3 node_position = glm::vec3(node_transform.gmodel[3]);

	if (parent_matrix != glm::mat4(1.0f))
	{
		glm::vec3 parent_position = glm::vec3(parent_matrix[3]);
		// TODO: think about what to do with debug drawing
		//debug_draw->drawDebugLine(parent_position, node_position);
	}

	for (auto child : node_hierarchy.children)
		debug_draw_skeleton(child, node_transform.gmodel);
}

CSkinningSystem::CSkinningSystem(render::IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CSkinningSystem::__Create()
{
	TRACY_PROFILE_FUNCTION();
}

void CSkinningSystem::__Update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto root = EntityManager::GetRoot();
	auto& root_component = EntityManager::GetComponent<FRootComponent>(root);

	auto view = EntityManager::View<FTransformComponent, FBaseComponent, FMeshComponent>();
	for (auto [entity, transform, base, mesh] : view.each())
	{
		if (!base.enabled || mesh.skin < 0)
			continue;

		auto& skin = root_component.skins[mesh.skin];

#ifndef BUILD_WITHOUT_DEBUGGIND
		debug_draw_skeleton(skin.root);
#endif

		if (skin.jointMatrices.size() != skin.joints.size())
		{
			skin.jointMatrices.resize(skin.joints.size());

			skin.indices.resize(skin.joints.size());
			std::iota(skin.indices.begin(), skin.indices.end(), 0ull);
		}


		// Calculate joint matrices
		auto inv_transform = glm::inverse(transform.gmodel);
#ifdef USE_STD_MULTITHREADING
		std::for_each(std::execution::par, skin.indices.begin(), skin.indices.end(),
			[&](const auto& idx)
#else
		for (const auto& idx : skin.indices)
#endif
		{
			auto& jtransform = EntityManager::GetComponent<FTransformComponent>(skin.joints[idx]);
			skin.jointMatrices[idx] = jtransform.gmodel * skin.inverseBindMatrices[idx];
			skin.jointMatrices[idx] = inv_transform * skin.jointMatrices[idx];
		}
#ifdef USE_STD_MULTITHREADING
		);
#endif
	}
	// TODO: RENDER WORLD
	//auto& render_world = m_pRenderInterface->GetRenderWorld();
	//auto& world_stats = render_world->GetStats();

	//for (u64 idx = 0ull; idx < root_component.skins.size(); ++idx)
	//{
	//	auto& skin = root_component.skins[idx];
	//	root_component.skinRemap[idx] = world_stats.matricesCount;
	//	render_world->AddMatrices(skin.jointMatrices);
	//}
}