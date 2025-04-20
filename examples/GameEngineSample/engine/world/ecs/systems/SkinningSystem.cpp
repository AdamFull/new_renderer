#include "SkinningSystem.h"

#include "../../EntityManager.h"

#include "../components/BaseComponent.h"
#include "../components/HierarchyComponent.h"
#include "../components/MeshComponent.h"
#include "../components/RootComponent.h"
#include "../components/TransformComponent.h"

#include <edge/util/utl_profiler.h>

#include <execution>

//#define USE_STD_MULTITHREADING

namespace engine::world::ecs {
	void debug_draw_skeleton(entt::entity node, glm::mat4 parent_matrix = glm::mat4(1.f)) {
		if (!EntityManager::is_valid(node)) {
			return;
		}

		auto& node_transform = EntityManager::get_component<TransformComponent>(node);
		auto& node_hierarchy = EntityManager::get_component<HierarchyComponent>(node);

		glm::vec3 node_position = glm::vec3(node_transform.gmodel[3]);

		if (parent_matrix != glm::mat4(1.0f)) {
			glm::vec3 parent_position = glm::vec3(parent_matrix[3]);
			// TODO: think about what to do with debug drawing
			//debug_draw->drawDebugLine(parent_position, node_position);
		}

		for (auto child : node_hierarchy.children) {
			debug_draw_skeleton(child, node_transform.gmodel);
		}
	}

	SkinningSystem::SkinningSystem(render::IRenderer& renderer) :
		render_world{ renderer.get_render_world() } {
	}

	void SkinningSystem::__create() {
	}

	void SkinningSystem::__update(float fDt) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto root = EntityManager::get_root();
		auto& root_component = EntityManager::get_component<RootComponent>(root);

		auto view = EntityManager::view<TransformComponent, BaseComponent, MeshComponent>();
		for (auto [entity, transform, base, mesh] : view.each()) {
			if (!base.enabled || mesh.skin < 0) {
				continue;
			}

			auto& skin = root_component.skins[mesh.skin];

#ifndef BUILD_WITHOUT_DEBUGGIND
			debug_draw_skeleton(skin.root);
#endif

			if (skin.jointMatrices.size() != skin.joints.size()) {
				skin.jointMatrices.resize(skin.joints.size());

				skin.indices.resize(skin.joints.size());
				std::iota(skin.indices.begin(), skin.indices.end(), 0ull);
			}


			// Calculate joint matrices
			auto inv_transform = glm::inverse(transform.gmodel);
#ifdef USE_STD_MULTITHREADING
			std::for_each(std::execution::par, skin.indices.begin(), skin.indices.end(),
				[&](const auto& idx) {
#else
			for (const auto& idx : skin.indices) {
#endif
				auto& jtransform = EntityManager::get_component<TransformComponent>(skin.joints[idx]);
				skin.jointMatrices[idx] = jtransform.gmodel * skin.inverseBindMatrices[idx];
				skin.jointMatrices[idx] = inv_transform * skin.jointMatrices[idx];
			}
#ifdef USE_STD_MULTITHREADING
				);
#endif
		}
		// TODO: RENDER WORLD
		//auto& render_world = m_pRenderInterface->get_render_world();
		//auto& world_stats = render_world->GetStats();

		//for (uint64_t idx = 0ull; idx < root_component.skins.size(); ++idx)
		//{
		//	auto& skin = root_component.skins[idx];
		//	root_component.skinRemap[idx] = world_stats.matricesCount;
		//	render_world->AddMatrices(skin.jointMatrices);
		//}
	}
}