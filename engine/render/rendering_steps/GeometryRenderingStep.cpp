#include "GeometryRenderingStep.h"

#include <unordered_set>

#include <engine/render/ShaderObject.h>
#include <engine/profiler/Profiler.h>

#include <logger/logger.h>

using namespace engine::render;

std::string attr_type_to_definition(EVertexAttributeType type)
{
	switch (type)
	{
	case engine::render::EVertexAttributeType::eColor: return "HAS_COLOR"; break;
	case engine::render::EVertexAttributeType::eTexCoord: return "HAS_TEXCOORD"; break;
	case engine::render::EVertexAttributeType::eNormal: return "HAS_NORMAL"; break;
	case engine::render::EVertexAttributeType::eTangent: return "HAS_TANGENT"; break;
	case engine::render::EVertexAttributeType::eJoint:
	case engine::render::EVertexAttributeType::eWeight: return "HAS_SKIN"; break;
	}
}

CGeometryRenderingStep::CGeometryRenderingStep(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CGeometryRenderingStep::_create(const FRenderingStepCreateInfo& create_info)
{
	TRACY_PROFILE_FUNCTION();

	auto& rendering_ci = create_info.rendering;

	m_baseShaderName = rendering_ci.shaderName;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_beginRenderingInfo.name = std::format("geometry:{}", rendering_ci.shaderName);
#endif
}

void CGeometryRenderingStep::_execute(float dt)
{
	TRACY_PROFILE_FUNCTION();

	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->beginRendering(m_beginRenderingInfo);

	auto& rect = m_beginRenderingInfo.renderArea;
	render_context->setViewport(static_cast<float>(rect.size.x), static_cast<float>(rect.size.y), m_beginRenderingInfo.flipViewport);
	render_context->setScissorRect(0, 0, rect.size.x, rect.size.y);

	auto& render_world = m_pRenderInterface->getRenderWorld();

	auto& batch_ranges = render_world->getGeometryRanges(RENDER_GEOMETRY_CULLER_SCENE_TYPE);

	size_t drawcall_id{ 0ull };
	IVertexHandler* last_vbo{ nullptr };
	for (const auto& batch_range : batch_ranges)
	{
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
				// TODO: this code looks very ugly as i think
				// Create shader for vertex declaration if not exists
				auto& shader = getShaderByVBO(last_vbo);

				if (was_vbo_rebinded)
				{
					shader->flushDescriptors();
					shader->bind(m_beginRenderingInfo.queue);
				}

				auto& mesh_data = shader->getConstantBlock("meshData");
				if (mesh_data)
				{
					mesh_data->set("callID", static_cast<uint32_t>(drawcall_id));
					mesh_data->set("batchID", static_cast<uint32_t>(batch_id));
				}

				// Bind vbo and flush descriptors, then bind and draw

				shader->flushConstants(m_beginRenderingInfo.queue);

				render_context->draw(m_beginRenderingInfo.queue, batch.command);
				drawcall_id += batch.command.count;
			}
		}
	}

	render_context->endRendering(m_beginRenderingInfo);
}

const std::unique_ptr<IShaderObject>& CGeometryRenderingStep::getShaderByVBO(const IVertexHandler* vbo)
{
	auto vbo_hash = vbo->getHash();
	auto& shader = m_shaders[vbo_hash];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->getDefinitions();

		FGraphicsShaderCreateInfo shader_ci{};
		shader_ci.cullMode = m_beginRenderingInfo.flipViewport ? SHADER_CULL_MODE_BACK_BIT : SHADER_CULL_MODE_FRONT_BIT;
		shader_ci.frontFace = m_beginRenderingInfo.flipViewport ? SHADER_FRONT_FACE_CW : SHADER_FRONT_FACE_CCW;
		shader_ci.primitiveTopology = SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		shader_ci.colorAttachments = m_beginRenderingInfo.colorAttachments;
		shader_ci.depthAttachment = m_beginRenderingInfo.depthAttachment;
		shader_ci.stencilAttachment = m_beginRenderingInfo.stencilAttachment;

		auto& render_packet = m_pRenderInterface->getRenderPacket();
		// TODO: i think i can calculate number of calls
		//shader_ci.usageCount = std::max(render_packet->stats.materialCount, render_packet->stats.primitiveCount);
		shader_ci.usageCount = render_packet->stats.perTypeVBOCount[vbo_hash];
		shader_ci.vertexDeclaration = vbo->getVertexDeclaration();

		shader->create(shader_name, shader_ci);
	}

	return shader;
}