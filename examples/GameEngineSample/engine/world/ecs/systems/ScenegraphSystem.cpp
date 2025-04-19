#include "ScenegraphSystem.h"

#include <glm/gtx/matrix_decompose.hpp>

#include "../components/BaseComponent.h"
#include "../components/HierarchyComponent.h"
#include "../components/TransformComponent.h"
#include "../../EntityManager.h"

#include <entt/entt.hpp>

#include <edge/util/utl_profiler.h>

namespace engine::world::ecs {
	ScenegraphSystem::ScenegraphSystem(render::IRenderer& renderer) :
		render_world{ renderer.GetRenderWorld() } {
		EntityManager::register_component<RootComponent>(EComponentFlagBits::eHiddenInEditor);
		EntityManager::register_component<BaseComponent>(EComponentFlagBits::eHiddenInEditor);
		EntityManager::register_component<HierarchyComponent>(EComponentFlagBits::eHiddenInEditor);
		EntityManager::register_component<TransformComponent>();

		auto& registry = EntityManager::get_registry();
	}

	void ScenegraphSystem::__create() {
	}

	void ScenegraphSystem::__update(float fDt) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto& registry = EntityManager::get_registry();
		auto root = EntityManager::get_root();
		if (!EntityManager::is_valid(root)) {
			return;
		}

		build_hierarchy(root);
	}

	void ScenegraphSystem::build_hierarchy(entt::entity node) {
		auto& base = EntityManager::get_component<BaseComponent>(node);
		auto& transform = EntityManager::get_component<TransformComponent>(node);
		auto& hierarchy = EntityManager::get_component<HierarchyComponent>(node);

		// Check is update disabled for branch
		if (!base.enabled)
			return;

		transform.oldmodel = transform.gmodel;
		init_matrix(&transform);

		auto parent = hierarchy.parent;
		if (EntityManager::is_valid(parent)) {
			auto& ptransform = EntityManager::get_component<TransformComponent>(parent);
			transform.gmodel = ptransform.gmodel * transform.gmodel;
		}

		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(transform.gmodel, transform.gscale, transform.grotation, transform.gposition, skew, perspective);

		transform.grotation = glm::normalize(transform.grotation);

		for (const auto& child : hierarchy.children) {
			build_hierarchy(child);
		}
	}

	void ScenegraphSystem::init_matrix(TransformComponent* transform) {
		transform->gmodel = glm::translate(glm::mat4(1.0f), transform->lposition) 
			* glm::mat4(transform->lrotation) 
			* glm::scale(glm::mat4(1.0f), transform->lscale) 
			* transform->lmodel;
	}

	void ScenegraphSystem::init_matrices(entt::registry* registry, entt::entity node) {
	}

	void ScenegraphSystem::calculate_matrices(entt::registry* registry, entt::entity node) {
	}
}