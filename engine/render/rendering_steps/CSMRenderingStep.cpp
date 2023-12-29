#include "CSMRenderingStep.h"

#include <unordered_set>

#include <engine/render/ShaderObject.h>
#include <engine/profiler/Profiler.h>

#include <logger/logger.h>

using namespace engine::render;

CCascadedShadowMapsRenderingStep::CCascadedShadowMapsRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
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

	auto& render_packet = m_pRenderInterface->getRenderPacket();
	auto& dlights = render_packet->dlights;
	size_t shadow_count = dlights.shadows->size();

	if (!shadow_count)
		return;

	m_pRenderContext->beginRendering(m_beginRenderingInfo);

	auto& rect = m_beginRenderingInfo.renderArea;
	m_pRenderContext->setViewport(static_cast<float>(rect.size.x), static_cast<float>(rect.size.y), m_beginRenderingInfo.flipViewport);
	m_pRenderContext->setScissorRect(0, 0, rect.size.x, rect.size.y);
	
	auto& materials = render_packet->materials;
	auto& nodes = render_packet->nodes;
	auto& skins = render_packet->skins;
	
	IVertexHandler* last_vbo{ nullptr };
	for (size_t batch_id = 0ull; batch_id < render_packet->sceneDrawCalls.size(); ++batch_id)
	{
		const auto& batch = render_packet->sceneDrawCalls.at(batch_id);

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
			if(shadow_data)
			{
				shadow_data->set("batchID", static_cast<uint32_t>(batch_id));
			}
	
			if (was_vbo_rebinded)
			{
				if (materials)
				{
					auto& buffer = materials->getBufferObject();
					size_t handle = buffer->handle();
					shader->bindBuffer("MaterialStorage", handle);
				}
	
				if (nodes)
				{
					auto& buffer = nodes->getBufferObject();
					size_t handle = buffer->handle();
					shader->bindBuffer("NodeStorage", handle);
				}
	
				if (skins)
				{
					auto& buffer = skins->getBufferObject();
					size_t handle = buffer->handle();
					shader->bindBuffer("SkinningJoints", handle);
				}
	
				if (dlights.shadows)
				{
					auto& buffer = dlights.shadows->getBufferObject();
					size_t handle = buffer->handle();
					shader->bindBuffer("CascadedShadows", handle);
				}
	
				if (render_packet->plights.shadows)
				{
					auto& buffer = render_packet->plights.shadows->getBufferObject();
					size_t handle = buffer->handle();
					shader->bindBuffer("OmniShadows", handle);
				}
	
				if (render_packet->slights.shadows)
				{
					auto& buffer = render_packet->slights.shadows->getBufferObject();
					size_t handle = buffer->handle();
					shader->bindBuffer("SpotShadows", handle);
				}

				if (render_packet->gpuBatches)
				{
					auto& buffer = render_packet->gpuBatches->getBufferObject();
					size_t batches = buffer->handle();
					shader->bindBuffer("GeometryBatches", batches);
				}
	
				shader->flushDescriptors();
				shader->bind(m_beginRenderingInfo.queue);
			}
	
			for (size_t idx = 0ull; idx < shadow_count; ++idx)
			{
				for (size_t layer = 0ull; layer < cascade_shadow_map_cascade_count; ++layer)
				{
					if (shadow_data)
					{
						shadow_data->set("shadowIndex", static_cast<uint32_t>(idx));
						shadow_data->set("layer", static_cast<int32_t>(layer));
					}
	
					shader->flushConstants(m_beginRenderingInfo.queue);
			
					m_pRenderContext->draw(m_beginRenderingInfo.queue, batch.command);
				}
			}
		}
	}

	m_pRenderContext->endRendering(m_beginRenderingInfo);
}

const std::unique_ptr<IShaderObject>& CCascadedShadowMapsRenderingStep::getShaderByVBO(const IVertexHandler* vbo)
{
	auto vbo_hash = vbo->getHash();
	auto& shader = m_shaders[vbo_hash];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface, m_pRenderContext);

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