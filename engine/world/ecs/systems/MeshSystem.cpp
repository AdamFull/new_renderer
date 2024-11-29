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
			uint32_t GetSortKeyByDepth(render::IRenderWorld& render_world, uint32_t camera_index, uint32_t node_index)
			{
				ZoneScoped;

				const auto& camera = render_world.GetRenderCamera(camera_index);
				const auto& node = render_world.GetNode(node_index);

				const auto& world_matrix = render_world.GetNodeMatrix(node_index, 0u);
				const auto& view_matrix = render_world.GetCameraMatrix(camera_index, shlib::view_matrix_offset);

				glm::vec4 view_space_pos = view_matrix * glm::vec4(glm::vec3(world_matrix[3]), 1.f);

				float normalized_depth = (view_space_pos.z - camera.z_near) / (camera.z_far - camera.z_near);
				// Is reverse z
				if (false)
				{
					normalized_depth = 1.0f - normalized_depth;
				}

				normalized_depth = glm::clamp(glm::abs(normalized_depth), 0.0f, 1.0f);

				return static_cast<uint32_t>(normalized_depth * 0xFFFFFFFF);
			}

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

				PrepareDrawCommands(2u);
				PrepareDrawCommands(0u);
				PrepareDrawCommands(1u);
			}

			void CMeshSystem::PrepareDrawCommands(uint32_t alpha_mode)
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
						const auto& mesh = render_world.GetMesh(primitive.mesh);
						const auto& material = render_world.GetMaterial(mesh.material);
						if (material.alphaMode != alpha_mode)
							continue;

						shlib::DrawDescription draw_command{};
						draw_command.vbo_index = primitive.vbo;
						draw_command.node_index = render_world.GetNodeId(transform.node_index);
						draw_command.mesh_index = primitive.mesh;
						draw_command.sorting_order = GetSortKeyByDepth(render_world, 0u, transform.node_index);

						render_world.AddGeometryDraw(draw_command);
					}
				}
			}
		}
	}
}