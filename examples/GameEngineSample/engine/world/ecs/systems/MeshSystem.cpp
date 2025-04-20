#include "MeshSystem.h"

#include "../../EntityManager.h"
#include "../components/BaseComponent.h"
#include "../components/MeshComponent.h"
#include "../components/RootComponent.h"
#include "../components/TransformComponent.h"

//#include "../../render/DebugDraw/DebugDraw.h"

#include <edge/util/utl_profiler.h>

#include "../../../rendering/Globals.h"

namespace engine::world::ecs {
	MeshSystem::MeshSystem(render::IRenderer& renderer) :
		render_world{ renderer.get_render_world() } {
		EntityManager::register_component<MeshComponent>();
	}

	void MeshSystem::__create() {
	}

	void MeshSystem::__update(float fDt) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto root = EntityManager::get_root();
		auto& root_component = EntityManager::get_component<RootComponent>(root);

		auto view = EntityManager::view<TransformComponent, BaseComponent, MeshComponent>();
		for (auto [entity, transform, base, mesh_component] : view.each()) {
			if (!base.visible)
				continue;

			//if (auto found = root_component.skinRemap.find(static_cast<uint32_t>(mesh_component.skin)); found != root_component.skinRemap.end())
			//	scene_node.skeletonID = found->second;

			if (mesh_component.skin != -1) {
				const auto& skin = root_component.skins[mesh_component.skin];
				render_world.update_joint_matrices(mesh_component.mesh, skin.jointMatrices);
			}

			render_world.update_mesh_matrices(mesh_component.mesh, transform.gmodel);
			render_world.add_geometry_draw(render::main_camera_index, mesh_component.mesh);
		}
	}
}