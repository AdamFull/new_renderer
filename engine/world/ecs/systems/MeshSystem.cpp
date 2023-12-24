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

	std::vector<FGeometryRawCommand> draw_primitives;

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

			auto& draw_data = draw_primitives.emplace_back();
			draw_data.vboID = primitive.vboId;
			draw_data.materialID = primitive.materialId;
			draw_data.instanceID = node;

			draw_data.range.beginIndex = lod.beginIndex;
			draw_data.range.indexCount = lod.indexCount;
			draw_data.range.beginVertex = lod.beginVertex;
			draw_data.range.vertexCount = lod.indexCount;

			draw_data.bounds.min = primitive.dims.min;
			draw_data.bounds.max = primitive.dims.max;
		}
	}

	// Sort drawings by vbo, material and node
	std::sort(draw_primitives.begin(), draw_primitives.end(), [](const FGeometryRawCommand& a, const FGeometryRawCommand& b)
		{
			if (a.vboID == b.vboID)
			{
				if(a.materialID == b.materialID)
					return a.instanceID < b.instanceID;

				return a.materialID < b.materialID;
			}

			return a.vboID < b.vboID;
		});

	for(const auto& draw_data : draw_primitives)
		m_pRenderInterface->drawPrimitive(draw_data);
}