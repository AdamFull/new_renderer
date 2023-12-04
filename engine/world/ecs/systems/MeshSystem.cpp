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

	std::vector<FGeometryDrawData> geometry_list{};

	auto view = registry->view<FTransformComponent, FBaseComponent, FMeshComponent>();
	for (auto [entity, transform, base, mesh] : view.each())
	{
		if (!base.visible)
			continue;

		auto node = static_cast<uint32_t>(entity);
		render_packet->nodes[node] = transform.gmodel;

		for (const auto& primitive : mesh.primitives)
		{
			auto& lod = primitive.lods[0ull];

			auto& draw_data = geometry_list.emplace_back();
			draw_data.vboId = primitive.vboId;
			draw_data.materialId = primitive.materialId;
			draw_data.node = node;
			draw_data.skin = mesh.skin;
			draw_data.range.beginIndex = lod.beginIndex;
			draw_data.range.indexCount = lod.indexCount;
			draw_data.range.beginVertex = lod.beginVertex;
			draw_data.range.vertexCount = lod.indexCount;
		}
	}

	for (const auto& skin : root_component.skins)
		render_packet->skins.emplace_back(skin.jointMatrices);

	createGeometryBatches(geometry_list, render_packet->sceneBatches, GEOMETRY_BATCHER_MERGE_DRAWCALLS_FLAG_BIT);
}