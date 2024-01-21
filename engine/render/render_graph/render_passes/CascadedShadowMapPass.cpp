#include "CascadedShadowMapPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CCascadedShadowMapPass::CCascadedShadowMapPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CCascadedShadowMapPass::init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::init(pass_ci);

	m_baseShaderName = pass_ci.shader;
	m_layerCount = cascade_shadow_map_cascade_count;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("csm:{}", m_baseShaderName);
#endif
}

void CCascadedShadowMapPass::build(const FPassBuildInfo& build_info)
{
	IRenderPass::build(build_info);
}

void CCascadedShadowMapPass::_render(const FRenderFrameInfo& frame_info)
{
	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& world_stats = render_world->getStats();

	if (!world_stats.cascadedShadowCount)
		return;

	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->setScissorRect(0, 0, frame_info.renderRect.size.x, frame_info.renderRect.size.y);

	auto& batch_ranges = render_world->getGeometryRanges(ERenderGeometryCullerType::eCascadedShadow);

	IVertexHandler* last_vbo{ nullptr };

	for (u64 range_id = 0ull; range_id < batch_ranges.size(); ++range_id)
	{
		u64 shadow_index = range_id / cascade_shadow_map_cascade_count;

		auto& batch_range = batch_ranges.at(range_id);
		for (u64 batch_id = batch_range.offset; batch_id < batch_range.offset + batch_range.count; ++batch_id)
		{
			const auto& batch = render_world->getGeometryBatch(batch_id);

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

				auto& shadow_data = shader->getConstantBlock("shadowData");
				if (shadow_data)
				{
					shadow_data->set("batchID", utl::cast<u32>(batch_id));
				}

				if (was_vbo_rebinded)
				{
					shader->flushDescriptors();
					shader->bind(m_queue);
				}

				// Draw shadfow
				if (shadow_data)
				{
					shadow_data->set("shadowIndex", utl::cast<u32>(shadow_index));
					shadow_data->set("layer", utl::cast<i32>(range_id));
				}

				shader->flushConstants(m_queue);

				render_context->draw(m_queue, batch.command);
			}
		}
	}
}

const std::unique_ptr<IShaderObject>& CCascadedShadowMapPass::getShaderByVBO(const IVertexHandler* vbo)
{
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto vbo_hash = vbo->getHash();
	auto& shader = m_shaders[vbo_hash];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->getDefinitions();

		FGraphicsShaderCreateInfo shader_ci{};
		shader_ci.cullMode = m_bFlipViewport ? EShaderCullModeFlagBits::eFront : EShaderCullModeFlagBits::eFront;
		shader_ci.frontFace = m_bFlipViewport ? EShaderFrontFace::eCW : EShaderFrontFace::eCCW;
		shader_ci.primitiveTopology = EVertexPrimitiveTopology::eTriangleList;

		auto& output_targets = m_renderResources[ERenderResourceGroupType::eColorWrite];
		for (u64 resource_id : output_targets)
		{
			if (resource_id == m_depthTarget || resource_id == m_stencilTarget)
				continue;

			auto& render_resource = render_graph->getRenderResource(resource_id);
			shader_ci.colorAttachments.emplace_back(render_resource.handle);
		}

		if (m_depthTarget != ~0ull)
		{
			auto& render_resource = render_graph->getRenderResource(m_depthTarget);
			shader_ci.depthAttachment = render_resource.handle;
		}

		if (m_stencilTarget != ~0ull)
		{
			auto& render_resource = render_graph->getRenderResource(m_stencilTarget);
			shader_ci.stencilAttachment = render_resource.handle;
		}

		auto& render_packet = m_pRenderInterface->getRenderPacket();
		// TODO: i think i can calculate number of calls
		shader_ci.usageCount = render_packet->stats.perTypeVBOCount[vbo_hash];
		shader_ci.vertexDeclaration = vbo->getVertexDeclaration();

		shader->create(shader_name, shader_ci);

		for (const auto& [bind_point, resource_id] : m_bakedBindings)
		{
			auto& render_resource = render_graph->getRenderResource(resource_id);
			if (render_resource.type & ERenderResourceTypeFlagBits::eAllImage)
				shader->bindTexture(bind_point.c_str(), render_resource.handle);
			else if (render_resource.type == ERenderResourceTypeFlagBits::eBuffer)
				shader->bindBuffer(bind_point.c_str(), render_resource.handle);
		}
	}

	return shader;
}