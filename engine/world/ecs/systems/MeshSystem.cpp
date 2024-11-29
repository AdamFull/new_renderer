#include "MeshSystem.h"

#include <engine/world/EntityManager.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/MeshComponent.h>
#include <engine/world/ecs/components/RootComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/render/DebugDraw/DebugDraw.h>

#include <tracy/Tracy.hpp>

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
				ZoneScoped;

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