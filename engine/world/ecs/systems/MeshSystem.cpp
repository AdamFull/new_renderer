#include "MeshSystem.h"

#include "../../EntityManager.h"
#include "../components/BaseComponent.h"
#include "../components/MeshComponent.h"
#include "../components/RootComponent.h"
#include "../components/TransformComponent.h"

#include <engine/render/DebugDraw/DebugDraw.h>

#include <edge/util/Profiler.h>

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

			void CMeshSystem::__Update(f32 fDt)
			{
				EDGE_PROFILE_ZONE_SCOPED;

				auto root = EntityManager::GetRoot();
				auto& root_component = EntityManager::GetComponent<FRootComponent>(root);

				auto view = EntityManager::View<FTransformComponent, FBaseComponent, FMeshComponent>();
				for (auto [entity, transform, base, mesh_component] : view.each())
				{
					if (!base.visible)
						continue;

					//if (auto found = root_component.skinRemap.find(utl::cast<u32>(mesh_component.skin)); found != root_component.skinRemap.end())
					//	scene_node.skeletonID = found->second;

					for (const auto& primitive : mesh_component.primitives)
					{
						render_world.AddGeometryDraw(2u, transform.node_index, primitive.mesh);
					}
				}
			}
		}
	}
}