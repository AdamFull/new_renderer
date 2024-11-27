#include "MeshSystem.h"

#include <engine/world/EntityManager.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/MeshComponent.h>
#include <engine/world/ecs/components/RootComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/render/DebugDraw/DebugDraw.h>

#include <tracy/Tracy.hpp>

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

		auto node = utl::cast<uint32_t>(entity);

		// Update node
		render_world.UpdateNode(node, transform.gmodel);

		//if (auto found = root_component.skinRemap.find(utl::cast<u32>(mesh_component.skin)); found != root_component.skinRemap.end())
		//	scene_node.skeletonID = found->second;

		for (const auto& primitive : mesh_component.primitives)
		{
			const auto& mesh = render_world.GetMesh(primitive.mesh);
			const auto& material = render_world.GetMaterial(mesh.material);
			if (material.alphaMode != alpha_mode)
				continue;

			shlib::DrawDescription draw_command{};
			draw_command.vboID = primitive.vbo;
			draw_command.nodeID = node;
			draw_command.meshID = primitive.mesh;

			render_world.AddGeometryDraw(draw_command);
		}
	}
}