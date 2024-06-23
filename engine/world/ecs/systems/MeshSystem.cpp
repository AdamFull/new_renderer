#include "MeshSystem.h"

#include <engine/world/SceneGraph.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/MeshComponent.h>
#include <engine/world/ecs/components/RootComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/render/DebugDraw/DebugDraw.h>

#include <engine/profiler/Profiler.h>

using namespace engine::render;
using namespace engine::world::ecs;

void prepare_meshes_to_draw(const std::unique_ptr<IRenderWorld>& render_world, const utl::vector<FMeshPrimitive>& primitives, utl::vector<FGeometryBatch>& draw_commands, u32 node, u32 alpha_mode)
{
	auto& world_stats = render_world->GetStats();

	utl::vector<FMesh> meshes{};
	for (const auto& primitive : primitives)
	{
		const auto& material = render_world->GetMaterial(primitive.materialId);
		if (material.alphaMode != alpha_mode)
			continue;

		auto& draw_command = draw_commands.emplace_back();
		draw_command.vboID = primitive.vboId.Get();
		draw_command.meshID = world_stats.meshCount + meshes.size();
		draw_command.nodeID = node;

		auto& mesh = meshes.emplace_back();
		mesh.materialID = primitive.materialId;
		mesh.lodOffset = primitive.vertexRange.firstLod;
		mesh.lodCount = primitive.vertexRange.lodCount;
		mesh.bounds.min = primitive.dims.min;
		mesh.bounds.max = primitive.dims.max;
	}

	render_world->AddMeshes(meshes);
}

void prepare_draw_commands(const std::unique_ptr<IRenderWorld>& render_world, engine::world::CSceneGraph* scene_graph, utl::vector<FGeometryBatch>& draw_commands, u32 alpha_mode)
{
	auto& registry = scene_graph->GetRegistry();
	auto root = scene_graph->GetRoot();
	auto& root_component = scene_graph->GetComponent<FRootComponent>(root);
	auto& world_stats = render_world->GetStats();

	auto view = registry->view<FTransformComponent, FBaseComponent, FMeshComponent>();
	for (auto [entity, transform, base, mesh_component] : view.each())
	{
		if (!base.visible)
			continue;

		auto node = utl::cast<u32>(entity);

		utl::vector<glm::mat4> matrices{ transform.gmodel, transform.oldmodel };
		FSceneNode scene_node{};
		scene_node.matrixOffset = world_stats.matricesCount;
		render_world->AddMatrices(matrices);

		if (auto found = root_component.skinRemap.find(utl::cast<u32>(mesh_component.skin)); found != root_component.skinRemap.end())
			scene_node.skeletonID = found->second;

		for (const auto& primitive : mesh_component.primitives)
		{
			const auto& material = render_world->GetMaterial(primitive.materialId);
			if (material.alphaMode != alpha_mode)
				continue;

			ImDbg::DrawBox3D(transform.gposition + primitive.dims.min * transform.gscale, transform.gposition + primitive.dims.max * transform.gscale, ImDbg::Color::Red);
		}

		auto remapped_idx = render_world->UpdateNode(node, scene_node);
		prepare_meshes_to_draw(render_world, mesh_component.primitives, draw_commands, remapped_idx, alpha_mode);
	}
}

CMeshSystem::CMeshSystem(CSceneGraph* scenegraph, render::IRenderInterface* render_interface)
{
	m_pSceneGraph = scenegraph;
	m_pRenderInterface = render_interface;
}

void CMeshSystem::__Create()
{
	TRACY_PROFILE_FUNCTION();
}

void CMeshSystem::__Update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& render_world = m_pRenderInterface->GetRenderWorld();

	utl::vector<FGeometryBatch> draw_commands{};
	prepare_draw_commands(render_world, m_pSceneGraph, draw_commands, utl::enum_cast(EAlphaMode::eOpaque));
	prepare_draw_commands(render_world, m_pSceneGraph, draw_commands, utl::enum_cast(EAlphaMode::eMask));
	prepare_draw_commands(render_world, m_pSceneGraph, draw_commands, utl::enum_cast(EAlphaMode::eBlend));

	render_world->AddDrawGeometry(draw_commands);
}