#include "MeshSystem.h"

#include <engine/world/SceneGraph.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/MeshComponent.h>
#include <engine/world/ecs/components/RootComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/profiler/Profiler.h>

using namespace engine::render;
using namespace engine::world::ecs;

void prepare_meshes_to_draw(const std::unique_ptr<IRenderWorld>& render_world, const utl::vector<FMeshPrimitive>& primitives, utl::vector<FGeometryBatch>& draw_commands, u32 node, u32 alpha_mode)
{
	auto& world_stats = render_world->getStats();

	utl::vector<FMesh> meshes{};
	for (const auto& primitive : primitives)
	{
		const auto& material = render_world->getMaterial(primitive.materialId);
		if (material.alphaMode != alpha_mode)
			continue;

		auto& draw_command = draw_commands.emplace_back();
		draw_command.vboID = primitive.vboId;
		draw_command.meshID = world_stats.meshCount + meshes.size();
		draw_command.nodeID = node;

		auto& mesh = meshes.emplace_back();
		mesh.materialID = primitive.materialId;
		mesh.lodOffset = primitive.vertexRange.firstLod;
		mesh.lodCount = primitive.vertexRange.lodCount;
		mesh.bounds.min = primitive.dims.min;
		mesh.bounds.max = primitive.dims.max;
	}

	render_world->addMeshes(meshes);
}

void prepare_draw_commands(const std::unique_ptr<IRenderWorld>& render_world, engine::world::CSceneGraph* scene_graph, utl::vector<FGeometryBatch>& draw_commands, u32 alpha_mode)
{
	auto& registry = scene_graph->getRegistry();
	auto root = scene_graph->getRoot();
	auto& root_component = scene_graph->getComponent<FRootComponent>(root);

	auto view = registry->view<FTransformComponent, FBaseComponent, FMeshComponent>();
	for (auto [entity, transform, base, mesh_component] : view.each())
	{
		if (!base.visible)
			continue;

		auto node = utl::cast<u32>(entity);

		FSceneNode scene_node{};
		scene_node.model = transform.gmodel;
		scene_node.oldModel = transform.oldmodel;

		if (auto found = root_component.skinRemap.find(utl::cast<u32>(mesh_component.skin)); found != root_component.skinRemap.end())
			scene_node.skeletonID = found->second;

		render_world->updateNode(node, scene_node);

		prepare_meshes_to_draw(render_world, mesh_component.primitives, draw_commands, node, alpha_mode);
	}
}

CMeshSystem::CMeshSystem(CSceneGraph* scenegraph, render::IRenderInterface* render_interface)
{
	m_pSceneGraph = scenegraph;
	m_pRenderInterface = render_interface;
}

void CMeshSystem::__create()
{
	TRACY_PROFILE_FUNCTION();
}

void CMeshSystem::__update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& render_world = m_pRenderInterface->getRenderWorld();

	utl::vector<FGeometryBatch> draw_commands{};
	prepare_draw_commands(render_world, m_pSceneGraph, draw_commands, utl::enum_cast(EAlphaMode::eOpaque));
	prepare_draw_commands(render_world, m_pSceneGraph, draw_commands, utl::enum_cast(EAlphaMode::eMask));
	prepare_draw_commands(render_world, m_pSceneGraph, draw_commands, utl::enum_cast(EAlphaMode::eBlend));

	auto& render_graph = m_pRenderInterface->getRenderGraph();
	auto& viewport = render_graph->getDefaultViewport();

	auto& registry = m_pSceneGraph->getRegistry();
	auto view = registry->view<FTransformComponent, FBaseComponent, FMeshComponent>();
	for (auto [entity, transform, base, mesh_component] : view.each())
	{
		if (!base.visible)
			continue;

		for (const auto& primitive : mesh_component.primitives)
		{
			glm::vec4 min_clip = viewport.viewProjection * transform.gmodel * glm::vec4(primitive.dims.min, 1.0f);
			glm::vec4 max_clip = viewport.viewProjection * transform.gmodel * glm::vec4(primitive.dims.max, 1.0f);

			glm::vec3 ndc_min = (min_clip / min_clip.w + 1.0f) * 0.5f;
			glm::vec3 ndc_max = (max_clip / max_clip.w + 1.0f) * 0.5f;

			glm::vec2 min_pos = glm::vec2(ndc_min) * viewport.size;
			glm::vec2 max_pos = glm::vec2(ndc_max) * viewport.size;

			glm::vec2 quad_size = glm::abs(max_pos - min_pos);

			f32 quad_area = quad_size.x * quad_size.y;
			f32 screen_area = viewport.size.x * viewport.size.y;
			float lod_ratio = glm::sqrt(screen_area / quad_area);
			u32 lod_level = glm::log2(glm::max(1.0f, lod_ratio * 12.f));

			lod_level++;
		}
	}

	render_world->addDrawGeometry(draw_commands);
}