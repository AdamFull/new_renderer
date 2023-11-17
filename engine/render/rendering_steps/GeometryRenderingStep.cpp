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
		shader_ci.cullMode = rendering_ci.flipViewport ? SHADER_CULL_MODE_BACK_BIT : SHADER_CULL_MODE_FRONT_BIT;
		shader_ci.frontFace = rendering_ci.flipViewport ? SHADER_FRONT_FACE_CW : SHADER_FRONT_FACE_CCW;
		shader_ci.primitiveTopology = SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		shader_ci.colorAttachments = rendering_ci.renderTargets;
		shader_ci.depthAttachment = rendering_ci.depthTarget;
		shader_ci.stencilAttachment = rendering_ci.stencilTarget;
		// TODO: Add attribute description
		shader_ci.vertexDeclaration.attributeDescriptions;

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

	if (m_pShader)
	{
		// Update all shader uniforms and other things. It should be before i bind shader
		m_pShader->flushDescriptors();
		m_pShader->flushConstants(m_beginRenderingInfo.queue);
		m_pShader->bind(m_beginRenderingInfo.queue);
	}
	else
	{
		// TODO: use shader from batch
	}

	// Here i can draw

	m_pRenderContext->endRendering(m_beginRenderingInfo);
}