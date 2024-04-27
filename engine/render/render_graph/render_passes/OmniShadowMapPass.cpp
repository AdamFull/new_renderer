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

	const auto& geometry_batches = render_world->getGeometryBatches();
	u64 drawcall_id{ 0ull };
	for (const auto& batch : geometry_batches)
	{
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

			auto& shadow_data = shader->getConstantBlock("shadowData");
			shadow_data->set("callID", utl::cast<u32>(drawcall_id));
			for (u64 shadow_index = 0ull; shadow_index < world_stats.omniShadowCount; ++shadow_index)
			{
				shadow_data->set("shadowIndex", utl::cast<u32>(shadow_index));
				shadow_data->set("lightIndex", utl::cast<u32>(0));
				for (u64 layer_index = 0ull; layer_index < 6ull; ++layer_index)
				{
					shadow_data->set("layer", utl::cast<i32>(layer_index));
					shader->flushConstants(m_queue);
					render_context->draw(m_queue, batch.command);
				}
			}
		}

		drawcall_id += batch.command.count;
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