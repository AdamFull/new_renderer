#include "MeshSystem.h"

#include <engine/world/EntityManager.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/MeshComponent.h>
#include <engine/world/ecs/components/RootComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/render/DebugDraw/DebugDraw.h>

#include <engine/profiler/Profiler.h>

using namespace engine::render;
using namespace engine::world;
using namespace engine::world::ecs;

CMeshSystem::CMeshSystem(render::IRenderer& renderer) :
	render_world{renderer.GetRenderWorld() }
{
	EntityManager::RegisterComponent<FMeshComponent>();
}

void CMeshSystem::__Create()
{
	TRACY_PROFILE_FUNCTION();
}

void CMeshSystem::__Update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	PrepareDrawCommands(2u);
	PrepareDrawCommands(0u);
	PrepareDrawCommands(1u);
}

void CMeshSystem::PrepareDrawCommands(uint32_t alpha_mode)
{
	auto root = EntityManager::GetRoot();
	auto& root_component = EntityManager::GetComponent<FRootComponent>(root);

	auto view = EntityManager::View<FTransformComponent, FBaseComponent, FMeshComponent>();
	for (auto [entity, transform, base, mesh_component] : view.each())
	{
		if (!base.visible)
			continue;

		auto node = utl::cast<uint32_t>(entity);

		// Update node
		render_world.UpdateNode(node, transform.gmodel);

		//if (auto found = root_component.skinRemap.find(utl::cast<u32>(mesh_component.skin)); found != root_component.skinRemap.end())
		//	scene_node.skeletonID = found->second;

		for (const auto& primitive : mesh_component.primitives)
		{
			const auto& material = render_world.GetMaterial(primitive.material);
			if (material.alphaMode != alpha_mode)
				continue;

			DrawCommand draw_command{};
			draw_command.node_index = node;

			draw_command.vertex_buffers = primitive.vbo;
			draw_command.material_index = primitive.material;

			draw_command.first_index = primitive.vertex_range.first_index;
			draw_command.index_count = primitive.vertex_range.index_count;
			draw_command.first_vertex = primitive.vertex_range.first_vertex;
			draw_command.vertex_count = primitive.vertex_range.vertex_count;

			render_world.AddGeometryDraw(draw_command);
		}
	}
}