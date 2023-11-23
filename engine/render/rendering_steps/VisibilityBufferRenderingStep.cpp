#include "VisibilityBufferRenderingStep.h"

#include <ShaderObject.h>
#include <logger/logger.h>

#include <unordered_set>

#include <profiler/Profiler.h>

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

CVisibilityBufferRenderingStep::CVisibilityBufferRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CVisibilityBufferRenderingStep::create(const FRenderingStepCreateInfo& create_info)
{
	TRACY_PROFILE_FUNCTION();

	auto& rendering_ci = create_info.rendering;

	// Save rendering info
	m_beginRenderingInfo.queue = rendering_ci.queue;
	m_beginRenderingInfo.colorAttachments = rendering_ci.renderTargets;
	m_beginRenderingInfo.depthAttachment = rendering_ci.depthTarget;
	m_beginRenderingInfo.stencilAttachment = rendering_ci.stencilTarget;
	m_beginRenderingInfo.flipViewport = rendering_ci.flipViewport;

	m_baseShaderName = rendering_ci.shaderName;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_beginRenderingInfo.name = std::format("visbuffer:{}", rendering_ci.shaderName);
#endif
}

void CVisibilityBufferRenderingStep::execute(float dt)
{
	TRACY_PROFILE_FUNCTION();

	auto render_resolution = m_pRenderContext->getRenderResolution();

	m_beginRenderingInfo.renderArea.offset.x = 0;
	m_beginRenderingInfo.renderArea.offset.y = 0;
	m_beginRenderingInfo.renderArea.size.x = render_resolution.x;
	m_beginRenderingInfo.renderArea.size.y = render_resolution.y;

	m_pRenderContext->beginRendering(m_beginRenderingInfo);

	m_pRenderContext->setViewport(static_cast<float>(render_resolution.x), static_cast<float>(render_resolution.y), m_beginRenderingInfo.flipViewport);
	m_pRenderContext->setScissorRect(0, 0, render_resolution.x, render_resolution.y);

	auto& render_packet = m_pRenderInterface->getRenderPacket();

	for (auto& batch : render_packet->sceneBatches)
	{
		auto& vbo = m_pRenderInterface->getVertexHandler(batch.vboId);
		if (vbo)
		{
			auto& vertex_decl = vbo->getVertexDeclaration();
			//auto vertex_hash = std::hash<FVertexDeclarationInfo>()(vertex_decl);
			auto vertex_hash = 0ull;

			// TODO: this code looks very ugly as i think
			// Create shader for vertex declaration if not exists
			auto& shader = m_shaders[vertex_hash];
			if (!shader)
			{
				shader = std::make_unique<CShaderObject>(m_pRenderInterface, m_pRenderContext);

				std::unordered_set<std::string> definitions{};
				for (const auto& attrib : vertex_decl.attributeDescriptions)
				{
					if (attrib.type == EVertexAttributeType::ePosition)
						continue;
					definitions.emplace(attr_type_to_definition(attrib.type));
				}

				// Generate shader name
				auto shader_name = m_baseShaderName;
				for (const auto& define : definitions)
					shader_name += ":" + define;

				FGraphicsShaderCreateInfo shader_ci{};
				shader_ci.cullMode = m_beginRenderingInfo.flipViewport ? SHADER_CULL_MODE_FRONT_BIT : SHADER_CULL_MODE_BACK_BIT;
				shader_ci.frontFace = m_beginRenderingInfo.flipViewport ? SHADER_FRONT_FACE_CW : SHADER_FRONT_FACE_CCW;
				shader_ci.primitiveTopology = SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				shader_ci.colorAttachments = m_beginRenderingInfo.colorAttachments;
				shader_ci.depthAttachment = m_beginRenderingInfo.depthAttachment;
				shader_ci.stencilAttachment = m_beginRenderingInfo.stencilAttachment;

				// TODO: i think i can calculate number of calls
				shader_ci.usageCount = 50u;
				shader_ci.vertexDeclaration = vertex_decl;

				shader->create(shader_name, shader_ci);
			}

			auto& node = render_packet->nodes.at(batch.node);

			{
				TRACY_PROFILE_SCOPE("Update UniformMeshData");

				auto& ubo = shader->getUniformBlock("UniformMeshData");
				ubo->set("model", node.model);
				ubo->set("view", render_packet->camera.view);
				ubo->set("projection", render_packet->camera.projection);
			}

			if (batch.skin >= 0)
			{
				auto& joints = shader->getUniformBlock("UniformJoints");
				auto& matrices = render_packet->skins.at(batch.skin);
				joints->set("matrices", matrices);
			}

			// Bind vbo and flush descriptors, then bind and draw
			shader->flushDescriptors();
			shader->flushConstants(m_beginRenderingInfo.queue);
			shader->bind(m_beginRenderingInfo.queue);

			for (const auto& range : batch.ranges)
			{
				FGeometryDrawInfo draw_info{};
				draw_info.range = range;
				draw_info.lastIndex = vbo->getLastIndex();
				draw_info.lastVertex = vbo->getLastVertex();
				vbo->bind(m_beginRenderingInfo.queue);

				m_pRenderContext->draw(m_beginRenderingInfo.queue, draw_info);
			}
		}
	}

	// Here i can draw

	m_pRenderContext->endRendering(m_beginRenderingInfo);
}