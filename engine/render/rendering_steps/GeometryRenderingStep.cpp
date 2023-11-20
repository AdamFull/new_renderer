#include "GeometryRenderingStep.h"

#include <ShaderObject.h>
#include <logger/logger.h>

using namespace engine::render;

CGeometryRenderingStep::CGeometryRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CGeometryRenderingStep::create(const FRenderingStepCreateInfo& create_info)
{
	auto& rendering_ci = create_info.rendering;

	// Geometry pass with included shader
	if (!rendering_ci.shaderName.empty())
	{
		// Create shader
		FGraphicsShaderCreateInfo shader_ci{};
		shader_ci.cullMode = rendering_ci.flipViewport ? SHADER_CULL_MODE_FRONT_BIT : SHADER_CULL_MODE_BACK_BIT;
		shader_ci.frontFace = SHADER_FRONT_FACE_CCW;
		shader_ci.primitiveTopology = SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		shader_ci.colorAttachments = rendering_ci.renderTargets;
		shader_ci.depthAttachment = rendering_ci.depthTarget;
		shader_ci.stencilAttachment = rendering_ci.stencilTarget;
		// TODO: Add attribute description
		shader_ci.vertexDeclaration.bindingDescriptions.emplace_back(FVertexDeclarationBindingDescription{ 0u, 48u });
		shader_ci.vertexDeclaration.attributeDescriptions.emplace_back(FVertexDeclarationAttributeDescription{ 0u, 0u, 106u, 0u });
		shader_ci.vertexDeclaration.attributeDescriptions.emplace_back(FVertexDeclarationAttributeDescription{ 2u, 0u, 103u, 12u });
		shader_ci.vertexDeclaration.attributeDescriptions.emplace_back(FVertexDeclarationAttributeDescription{ 3u, 0u, 106u, 20u });
		shader_ci.vertexDeclaration.attributeDescriptions.emplace_back(FVertexDeclarationAttributeDescription{ 4u, 0u, 109u, 32u });

		// TODO: if using global shader for all geometry, should get usage count from scene
		shader_ci.usageCount = 50u;

		// Make RHI shader object (more like reflection object)
		m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface, m_pRenderContext);
		m_pShader->create(rendering_ci.shaderName, shader_ci);

		// Bind default textures to shader
		for (auto& [binding_point, texture] : rendering_ci.shaderInputs)
			m_pShader->bindTexture(binding_point.c_str(), texture);		
	}

	// Save rendering info
	m_beginRenderingInfo.queue = rendering_ci.queue;
	m_beginRenderingInfo.colorAttachments = rendering_ci.renderTargets;
	m_beginRenderingInfo.depthAttachment = rendering_ci.depthTarget;
	m_beginRenderingInfo.stencilAttachment = rendering_ci.stencilTarget;
	m_beginRenderingInfo.flipViewport = rendering_ci.flipViewport;
}

void CGeometryRenderingStep::execute(float dt)
{
	auto render_resolution = m_pRenderContext->getRenderResolution();

	m_beginRenderingInfo.renderArea.offset.x = 0;
	m_beginRenderingInfo.renderArea.offset.y = 0;
	m_beginRenderingInfo.renderArea.size.x = render_resolution.x;
	m_beginRenderingInfo.renderArea.size.y = render_resolution.y;

	m_pRenderContext->beginRendering(m_beginRenderingInfo);

	m_pRenderContext->setViewport(static_cast<float>(render_resolution.x), static_cast<float>(render_resolution.y), m_beginRenderingInfo.flipViewport);
	m_pRenderContext->setScissorRect(0, 0, render_resolution.x, render_resolution.y);

	auto& render_packet = m_pRenderInterface->getRenderPacket();

	if (m_pShader)
	{
		for (auto& batch : render_packet->sceneBatches)
		{
			auto& node = render_packet->nodes.at(batch.node);

			auto& ubo = m_pShader->getUniformBlock("UBOMeshData");
			ubo->set("model", node.model);
			ubo->set("view", render_packet->camera.view);
			ubo->set("projection", render_packet->camera.projection);

			if (batch.skin >= 0)
			{
				auto& joints = m_pShader->getUniformBlock("SkinningData");
				auto& matrices = render_packet->skins.at(batch.skin);
				joints->set("matrices", matrices);
			}

			// Bind vbo and flush descriptors, then bind and draw
			m_pShader->flushDescriptors();
			m_pShader->flushConstants(m_beginRenderingInfo.queue);
			m_pShader->bind(m_beginRenderingInfo.queue);

			for (const auto& range : batch.ranges)
			{
				FGeometryDrawInfo draw_info{};
				draw_info.range = range;

				auto& vbo = m_pRenderInterface->getVertexHandler(batch.vboId);
				if (vbo)
				{
					draw_info.lastIndex = vbo->getLastIndex();
					draw_info.lastVertex = vbo->getLastVertex();
					vbo->bind(m_beginRenderingInfo.queue);
				}

				m_pRenderContext->draw(m_beginRenderingInfo.queue, draw_info);
			}
		}
	}
	else
	{
		// TODO: use shader from batch
	}

	// Here i can draw

	m_pRenderContext->endRendering(m_beginRenderingInfo);
}