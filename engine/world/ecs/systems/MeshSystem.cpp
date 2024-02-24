#include "MeshSystem.h"

#include <engine/world/SceneGraph.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/MeshComponent.h>
#include <engine/world/ecs/components/RootComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/profiler/Profiler.h>

using namespace engine::render;
using namespace engine::world::ecs;

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

	auto& registry = m_pSceneGraph->getRegistry();
	auto root = m_pSceneGraph->getRoot();
	auto& root_component = m_pSceneGraph->getComponent<FRootComponent>(root);
	auto& render_packet = m_pRenderInterface->getRenderPacket();
	auto& render_world = m_pRenderInterface->getRenderWorld();

	stl::vector<FGeometryRawBatch> draw_commands{};

	auto view = registry->view<FTransformComponent, FBaseComponent, FMeshComponent>();
	for (auto [entity, transform, base, mesh] : view.each())
	{
		if (!base.visible)
			continue;

		auto node = utl::cast<u32>(entity);

		FSceneNode scene_node{};
		scene_node.model = transform.gmodel;
		if(auto found = root_component.skinRemap.find(utl::cast<u32>(mesh.skin)); found != root_component.skinRemap.end())
			scene_node.skin = found->second;
		render_world->updateNode(node, scene_node);

		for (const auto& primitive : mesh.primitives)
		{
			auto& lod = primitive.lods[0ull];

			auto& draw_command = draw_commands.emplace_back();
			draw_command.callInfo.vboID = primitive.vboId;
			draw_command.callInfo.materialID = primitive.materialId;
			draw_command.callInfo.instanceID = node;
			
			draw_command.range.beginIndex = lod.firstIndex;
			draw_command.range.indexCount = lod.indexCount;
			draw_command.range.beginVertex = lod.firstVertex;
			draw_command.range.vertexCount = lod.indexCount;

			draw_command.range.bounds.min = primitive.dims.min;
			draw_command.range.bounds.max = primitive.dims.max;
		}
	}

	render_world->addDrawGeometry(draw_commands);
}