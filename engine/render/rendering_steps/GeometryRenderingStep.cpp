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

CGeometryRenderingStep::CGeometryRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
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

	m_pRenderContext->beginRendering(m_beginRenderingInfo);

	auto& rect = m_beginRenderingInfo.renderArea;
	m_pRenderContext->setViewport(static_cast<float>(rect.size.x), static_cast<float>(rect.size.y), m_beginRenderingInfo.flipViewport);
	m_pRenderContext->setScissorRect(0, 0, rect.size.x, rect.size.y);

	auto& render_packet = m_pRenderInterface->getRenderPacket();

	auto& materials = render_packet->materials;
	auto& nodes = render_packet->nodes;
	auto& skins = render_packet->skins;

	IVertexHandler* last_vbo{ nullptr };
	for (auto& batch : render_packet->sceneBatches)
	{
 		auto& vbo = m_pRenderInterface->getVertexHandler(batch.vboId);
		if (!last_vbo || vbo.get() != last_vbo)
		{
			last_vbo = vbo.get();
			last_vbo->bind(m_beginRenderingInfo.queue);
		}
		
		if (last_vbo)
		{
			// TODO: this code looks very ugly as i think
			// Create shader for vertex declaration if not exists
			auto& shader = getShaderByVBO(last_vbo);

			{
				TRACY_PROFILE_SCOPE("Update UniformMeshData");
			
				auto& ubo = shader->getUniformBlock("UniformMeshData");
				ubo->set("vboId", static_cast<uint32_t>(batch.vboId));
				ubo->set("objectId", batch.node);
				ubo->set("materialId", static_cast<uint32_t>(batch.materialId));
			}

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

			// Bind vbo and flush descriptors, then bind and draw
			shader->flushDescriptors();
			shader->flushConstants(m_beginRenderingInfo.queue);
			shader->bind(m_beginRenderingInfo.queue);

			FGeometryDrawInfo draw_info{};
			draw_info.lastIndex = last_vbo->getLastIndex();
			draw_info.lastVertex = last_vbo->getLastVertex();
			draw_info.ranges = batch.ranges;

			m_pRenderContext->draw(m_beginRenderingInfo.queue, draw_info);
		}
	}

	m_pRenderContext->endRendering(m_beginRenderingInfo);
}

const std::unique_ptr<IShaderObject>& CGeometryRenderingStep::getShaderByVBO(const IVertexHandler* vbo)
{
	auto& shader = m_shaders[vbo->getHash()];
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
		shader_ci.usageCount = render_packet->primitiveCount;
		shader_ci.vertexDeclaration = vbo->getVertexDeclaration();

		shader->create(shader_name, shader_ci);
	}

	return shader;
}