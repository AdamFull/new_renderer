#include "MeshSystem.h"

#include <world/SceneGraph.h>

#include <world/ecs/components/BaseComponent.h>
#include <world/ecs/components/MeshComponent.h>
#include <world/ecs/components/TransformComponent.h>

using namespace engine::render;
using namespace engine::world::ecs;

CMeshSystem::CMeshSystem(CSceneGraph* scenegraph, render::IRenderInterface* render_interface)
{
	m_pSceneGraph = scenegraph;
	m_pRenderInterface = render_interface;
}

void CMeshSystem::__create()
{

}

void CMeshSystem::__update(float fDt)
{
	auto& registry = m_pSceneGraph->getRegistry();
	auto& render_packet = m_pRenderInterface->getRenderPacket();

	FFrustum frustum{};
	frustum.update(render_packet->camera.view, render_packet->camera.projection);

	auto& geometry_list = render_packet->sceneGeometry;

	auto view = registry->view<FTransformComponent, FBaseComponent, FMeshComponent>();
	for (auto [entity, transform, base, mesh] : view.each())
	{
		if (!base.visible)
			continue;

		for (const auto& primitive : mesh.primitives)
		{
			//bool is_in_view = frustum.checkBox(transform.gposition + primitive.dims.min * transform.gscale, transform.gposition + primitive.dims.max * transform.gscale);
			bool is_in_view{ true };
			if (is_in_view)
			{
				auto& lod = primitive.lods[0ull];

				auto& draw_data = geometry_list.emplace_back();
				draw_data.vboId = primitive.vboId;
				draw_data.materialId = primitive.materialId;
				draw_data.skin = mesh.skin;
				draw_data.range.beginIndex = lod.beginIndex;
				draw_data.range.indexCount = lod.indexCount;
				draw_data.range.beginVertex = lod.beginVertex;
				draw_data.range.vertexCount = lod.indexCount;
			}
		}
	}

	auto batches = createGeometryBatches(geometry_list);
	return;
}