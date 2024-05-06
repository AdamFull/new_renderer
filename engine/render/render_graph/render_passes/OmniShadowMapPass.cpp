#include "OmniShadowMapPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

COmnilShadowMapPass::COmnilShadowMapPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void COmnilShadowMapPass::init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::init(pass_ci);

	m_baseShaderName = pass_ci.shader;
	m_layerCount = max_point_light_shadow_count * 6u;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("osm:{}", m_baseShaderName);
#endif
}

void COmnilShadowMapPass::build(const FPassBuildInfo& build_info)
{
	IRenderPass::build(build_info);
}

void COmnilShadowMapPass::_render(const FRenderFrameInfo& frame_info)
{
	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& world_stats = render_world->getStats();

	if (!world_stats.omniShadowCount)
		return;

	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->setScissorRect(0, 0, frame_info.renderRect.size.x, frame_info.renderRect.size.y);

	IVertexHandler* last_vbo{ nullptr };

	const auto& draw_infos = render_world->getDrawCommandByType(ERenderGeometryCullerType::eSpotShadow);
	const auto& geometry_batches = render_world->getGeometryBatches();

	for (u64 idx = 0ull; idx < draw_infos.size(); ++idx)
	{
		const auto& draw_info = draw_infos[idx];
		auto begin_range = geometry_batches.begin() + draw_info.first;
		auto end_range = begin_range + draw_info.count;

		u64 drawcall_id{ draw_info.cullerID * world_stats.maxDrawCommands };
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
				auto& shader = getShaderByVBO(last_vbo);
				if (was_vbo_rebinded)
				{
					shader->flushDescriptors();
					shader->bind(m_queue);
				}

				auto& shadow_data = shader->getConstantBlock(shader_bindings::shader_data);
				shadow_data->set(shader_bindings::call_id_label, utl::cast<u32>(drawcall_id));
				for (u64 shadow_index = 0ull; shadow_index < world_stats.omniShadowCount; ++shadow_index)
				{
					shadow_data->set(shader_bindings::shadow_index_label, utl::cast<u32>(shadow_index));
					shadow_data->set(shader_bindings::layer_label, utl::cast<i32>(idx));
					shader->flushConstants(m_queue);
					render_context->draw(m_queue, batch.command);
				}
			}

			drawcall_id += draw_info.drawCount;
		}
	}
}

const std::unique_ptr<IShaderObject>& COmnilShadowMapPass::getShaderByVBO(const IVertexHandler* vbo)
{
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto vbo_hash = vbo->getHash();
	auto& shader = m_shaders[vbo_hash];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->getDefinitions();

		auto front_face = m_bFlipViewport ? EShaderFrontFace::eCW : EShaderFrontFace::eCCW;

		makeShader(shader_name, shader, EShaderCullModeFlagBits::eFront, front_face, vbo);
		initShaderResources(shader);
	}

	return shader;
}