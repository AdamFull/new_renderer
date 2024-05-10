#include "GeometryPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CGeometryPass::CGeometryPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CGeometryPass::init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::init(pass_ci);

	m_baseShaderName = pass_ci.shader;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("geometry:{}", m_baseShaderName);
#endif
}

void CGeometryPass::build(const FPassBuildInfo& build_info)
{
	IRenderPass::build(build_info);
}

void CGeometryPass::postLoad()
{
	auto& render_world = m_pRenderInterface->getRenderWorld();
	m_cameraIndex = render_world->getDefaultCameraID();
}

void CGeometryPass::_render(const FRenderFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->setScissorRect(0, 0, frame_info.renderRect.size.x, frame_info.renderRect.size.y);

	auto& render_world = m_pRenderInterface->getRenderWorld();
	const auto& world_stats = render_world->getStats();

	const auto& draw_info = render_world->getDrawCommandByType(ERenderGeometryCullerType::eScene).front();
	const auto& geometry_batches = render_world->getGeometryBatches();

	auto begin_range = geometry_batches.begin() + draw_info.first;
	auto end_range = begin_range + draw_info.count;

	u64 drawcall_id{ draw_info.cullerID * world_stats.maxDrawCommands };
	IVertexHandler* last_vbo{ nullptr };
	for (auto it = begin_range; it != end_range; ++it)
	{
		const auto& batch = *it;

		bool was_vbo_rebinded{ false };
		auto& vbo = m_pRenderInterface->getVertexHandler(batch.vboID);
		if (!last_vbo || vbo.get() != last_vbo)
		{
			last_vbo = vbo.get();
			last_vbo->bind(m_queue);
			was_vbo_rebinded = true;
		}

		if (last_vbo)
		{
			// TODO: this code looks very ugly as i think
			// Create shader for vertex declaration if not exists
			auto& shader = getShaderByVBO(last_vbo);

			if (was_vbo_rebinded)
			{
				shader->flushDescriptors();
				shader->bind(m_queue);
			}

			auto& mesh_data = shader->getConstantBlock(shader_bindings::mesh_data);
			if (mesh_data)
			{
				mesh_data->set(shader_bindings::call_id_label, utl::cast<u32>(drawcall_id));
				mesh_data->set(shader_bindings::batch_id_label, utl::cast<u32>(0));
			}

			// Bind vbo and flush descriptors, then bind and draw

			shader->flushConstants(m_queue);

			render_context->draw(m_queue, batch.command);
			drawcall_id += draw_info.drawCount;
		}
	}
}

const std::unique_ptr<IShaderObject>& CGeometryPass::getShaderByVBO(const IVertexHandler* vbo)
{
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto vbo_hash = vbo->getHash();
	auto& shader = m_shaders[vbo_hash];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->getDefinitions();

		auto cull_mode = m_bFlipViewport ? EShaderCullModeFlagBits::eBack : EShaderCullModeFlagBits::eFront;
		auto front_face = m_bFlipViewport ? EShaderFrontFace::eCW : EShaderFrontFace::eCCW;

		makeShader(shader_name, shader, cull_mode, front_face, vbo);
		initShaderResources(shader);
	}

	return shader;
}