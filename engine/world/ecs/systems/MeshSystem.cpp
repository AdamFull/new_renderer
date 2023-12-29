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

void CMeshSystem::__update(float fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = m_pSceneGraph->getRegistry();
	auto root = m_pSceneGraph->getRoot();
	auto& root_component = m_pSceneGraph->getComponent<FRootComponent>(root);
	auto& render_packet = m_pRenderInterface->getRenderPacket();

	FFrustum frustum{};
	frustum.update(render_packet->camera.view, render_packet->camera.projection);

	auto& nodes = render_packet->nodes;

	std::vector<FGeometryRawBatch> draw_commands{};

	auto view = registry->view<FTransformComponent, FBaseComponent, FMeshComponent>();
	for (auto [entity, transform, base, mesh] : view.each())
	{
		if (!base.visible)
			continue;

		auto node = static_cast<uint32_t>(entity);

		FSceneNode scene_node{};
		scene_node.model = transform.gmodel;
		scene_node.skin = static_cast<uint32_t>(mesh.skin);
		nodes->insert(&scene_node, node);

		for (const auto& primitive : mesh.primitives)
		{
			auto& lod = primitive.lods[0ull];

			auto& draw_command = draw_commands.emplace_back();
			draw_command.callInfo.vboID = primitive.vboId;
			draw_command.callInfo.materialID = primitive.materialId;
			draw_command.callInfo.instanceID = node;
			
			draw_command.range.beginIndex = lod.beginIndex;
			draw_command.range.indexCount = lod.indexCount;
			draw_command.range.beginVertex = lod.beginVertex;
			draw_command.range.vertexCount = lod.indexCount;

			draw_command.range.bounds.min = primitive.dims.min;
			draw_command.range.bounds.max = primitive.dims.max;
		}
	}

	// Sort drawings by vbo, material and node
	std::sort(draw_commands.begin(), draw_commands.end(), [](const FGeometryRawBatch& a, const FGeometryRawBatch& b)
		{
			if (a.callInfo.vboID == b.callInfo.vboID)
			{
				if(a.callInfo.materialID == b.callInfo.materialID)
					return a.callInfo.instanceID < b.callInfo.instanceID;

				return a.callInfo.materialID < b.callInfo.materialID;
			}

			return a.callInfo.vboID < b.callInfo.vboID;
		});

	m_pRenderInterface->writeDrawData(draw_commands);
}