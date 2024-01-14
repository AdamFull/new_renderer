#include "CSMRenderingStep.h"

#include <unordered_set>

#include <engine/render/ShaderObject.h>
#include <engine/profiler/Profiler.h>

#include <logger/logger.h>

using namespace engine::render;

CCascadedShadowMapsRenderingStep::CCascadedShadowMapsRenderingStep(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CCascadedShadowMapsRenderingStep::_create(const FRenderingStepCreateInfo& create_info)
{
	TRACY_PROFILE_FUNCTION();

	auto& rendering_ci = create_info.rendering;

	m_baseShaderName = rendering_ci.shaderName;
	m_beginRenderingInfo.layerCount = cascade_shadow_map_cascade_count;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_beginRenderingInfo.name = std::format("csm:{}", rendering_ci.shaderName);
#endif
}

void CCascadedShadowMapsRenderingStep::_execute(float dt)
{
	TRACY_PROFILE_FUNCTION();

	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& world_stats = render_world->getStats();

	if (!world_stats.cascadedShadowCount)
		return;

	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->beginRendering(m_beginRenderingInfo);

	auto& rect = m_beginRenderingInfo.renderArea;
	render_context->setViewport(static_cast<float>(rect.size.x), static_cast<float>(rect.size.y), m_beginRenderingInfo.flipViewport);
	render_context->setScissorRect(0, 0, rect.size.x, rect.size.y);
	
	auto& batch_ranges = render_world->getGeometryRanges(RENDER_GEOMETRY_CULLER_CSM_TYPE);
	
	IVertexHandler* last_vbo{ nullptr };
	
	for (size_t range_id = 0ull; range_id < batch_ranges.size(); ++range_id)
	{
		size_t shadow_index = range_id / cascade_shadow_map_cascade_count;

		auto& batch_range = batch_ranges.at(range_id);
		for (size_t batch_id = batch_range.offset; batch_id < batch_range.offset + batch_range.count; ++batch_id)
		{
			const auto& batch = render_world->getGeometryBatch(batch_id);
	
			bool was_vbo_rebinded{ false };
			auto& vbo = m_pRenderInterface->getVertexHandler(batch.vboID);
			if (!last_vbo || vbo.get() != last_vbo)
			{
				last_vbo = vbo.get();
				last_vbo->bind(m_beginRenderingInfo.queue);
				was_vbo_rebinded = true;
			}
	
			if (last_vbo)
			{
				auto& shader = getShaderByVBO(last_vbo);
	
				auto& shadow_data = shader->getConstantBlock("shadowData");
				if (shadow_data)
				{
					shadow_data->set("batchID", static_cast<uint32_t>(batch_id));
				}
	
				if (was_vbo_rebinded)
				{
					shader->flushDescriptors();
					shader->bind(m_beginRenderingInfo.queue);
				}

				// Draw shadfow
				if (shadow_data)
				{
					shadow_data->set("shadowIndex", static_cast<uint32_t>(shadow_index));
					shadow_data->set("layer", static_cast<int32_t>(range_id));
				}
				
				shader->flushConstants(m_beginRenderingInfo.queue);
				
				render_context->draw(m_beginRenderingInfo.queue, batch.command);
			}
		}
	}

	render_context->endRendering(m_beginRenderingInfo);
}

const std::unique_ptr<IShaderObject>& CCascadedShadowMapsRenderingStep::getShaderByVBO(const IVertexHandler* vbo)
{
	auto vbo_hash = vbo->getHash();
	auto& shader = m_shaders[vbo_hash];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->getDefinitions();

		FGraphicsShaderCreateInfo shader_ci{};
		shader_ci.cullMode = m_beginRenderingInfo.flipViewport ? SHADER_CULL_MODE_FRONT_BIT : SHADER_CULL_MODE_FRONT_BIT;
		shader_ci.frontFace = m_beginRenderingInfo.flipViewport ? SHADER_FRONT_FACE_CW : SHADER_FRONT_FACE_CCW;
		shader_ci.primitiveTopology = SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		shader_ci.colorAttachments = m_beginRenderingInfo.colorAttachments;
		shader_ci.depthAttachment = m_beginRenderingInfo.depthAttachment;
		shader_ci.stencilAttachment = m_beginRenderingInfo.stencilAttachment;

		auto& render_packet = m_pRenderInterface->getRenderPacket();
		// TODO: i think i can calculate number of calls
		shader_ci.usageCount = render_packet->stats.perTypeVBOCount[vbo_hash];
		shader_ci.vertexDeclaration = vbo->getVertexDeclaration();

		shader->create(shader_name, shader_ci);
	}

	return shader;
}