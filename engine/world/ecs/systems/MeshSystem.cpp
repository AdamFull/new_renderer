#include "MeshSystem.h"

#include "../../EntityManager.h"
#include "../components/BaseComponent.h"
#include "../components/MeshComponent.h"
#include "../components/RootComponent.h"
#include "../components/TransformComponent.h"

#include <engine/render/DebugDraw/DebugDraw.h>

#include <edge/util/utl_profiler.h>

#include <engine/rendering/Globals.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			CMeshSystem::CMeshSystem(render::IRenderer& renderer) :
				render_world{ renderer.GetRenderWorld() }
			{
				EntityManager::RegisterComponent<FMeshComponent>();
			}

			void CMeshSystem::__Create()
			{
			}

			void CMeshSystem::__Update(float fDt)
			{
				EDGE_PROFILE_ZONE_SCOPED;

				auto root = EntityManager::GetRoot();
				auto& root_component = EntityManager::GetComponent<FRootComponent>(root);

				auto view = EntityManager::View<FTransformComponent, FBaseComponent, FMeshComponent>();
				for (auto [entity, transform, base, mesh_component] : view.each())
				{
					if (!base.visible)
						continue;

					//if (auto found = root_component.skinRemap.find(static_cast<uint32_t>(mesh_component.skin)); found != root_component.skinRemap.end())
					//	scene_node.skeletonID = found->second;

					if (mesh_component.skin != -1)
					{
						const auto& skin = root_component.skins[mesh_component.skin];
						render_world.UpdateJointMatrices(mesh_component.mesh, skin.jointMatrices);
					}

					render_world.UpdateMeshMatrices(mesh_component.mesh, transform.gmodel);
					render_world.AddGeometryDraw(render::main_camera_index, mesh_component.mesh);
				}
			}
		}
	}
}