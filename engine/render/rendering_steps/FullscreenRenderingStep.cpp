#include "FullscreenRenderingStep.h"

#include <ShaderObject.h>
#include <logger/logger.h>

using namespace engine::render;

CFullscreenRenderingStep::CFullscreenRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CFullscreenRenderingStep::create(const FRenderingStepCreateInfo& create_info)
{
	auto& rendering_ci = create_info.rendering;

	if (rendering_ci.shaderName.empty())
		log_error("No shader name provided for fullscreen rendering step creation.");

	// Create shader
	FGraphicsShaderCreateInfo shader_ci{};
	shader_ci.cullMode = rendering_ci.flipViewport ? SHADER_CULL_MODE_BACK_BIT : SHADER_CULL_MODE_FRONT_BIT;
	shader_ci.frontFace = rendering_ci.flipViewport ? SHADER_FRONT_FACE_CW : SHADER_FRONT_FACE_CCW;
	shader_ci.primitiveTopology = SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	shader_ci.colorAttachments = rendering_ci.renderTargets;
	shader_ci.depthAttachment = rendering_ci.depthTarget;
	shader_ci.stencilAttachment = rendering_ci.stencilTarget;

	auto shader_id = m_pRenderInterface->createShader(rendering_ci.shaderName, shader_ci);
	auto shader_reflection = m_pRenderContext->getShaderReflection(shader_id);

	// Make RHI shader object (more like reflection object)
	m_pShader = std::make_unique<CShaderObject>();
	m_pShader->create(shader_id, shader_reflection);

	// Bind default textures to shader
	for (auto& [binding_point, texture] : rendering_ci.shaderInputs)
		m_pShader->bindTexture(binding_point.c_str(), texture);

	// Save rendering info
	m_beginRenderingInfo.queue = rendering_ci.queue;
	m_beginRenderingInfo.colorAttachments = rendering_ci.renderTargets;
	m_beginRenderingInfo.depthAttachment = rendering_ci.depthTarget;
	m_beginRenderingInfo.stencilAttachment = rendering_ci.stencilTarget;
	m_beginRenderingInfo.flipViewport = rendering_ci.flipViewport;
}

void CFullscreenRenderingStep::execute(float dt)
{
	// TODO: Push profiling scope here (tracy and push debug label to api). As name use shader name or add stage name for future

	auto render_resolution = m_pRenderContext->getRenderResolution();

	m_beginRenderingInfo.renderArea.offset.x = 0;
	m_beginRenderingInfo.renderArea.offset.y = 0;
	m_beginRenderingInfo.renderArea.size.x = render_resolution.x;
	m_beginRenderingInfo.renderArea.size.y = render_resolution.y;

	m_pRenderContext->beginRendering(m_beginRenderingInfo);

	m_pRenderContext->setViewport(static_cast<float>(render_resolution.x), static_cast<float>(render_resolution.y), m_beginRenderingInfo.flipViewport);
	m_pRenderContext->setScissorRect(0, 0, render_resolution.x, render_resolution.y);

	// Here i can update uniforms

	// Update all shader uniforms and other things. It should be before i bind shader
	auto resource_commit = m_pShader->getResourceCommit();
	m_pRenderContext->updateShaderResources(resource_commit);
	m_pRenderContext->flushConstantRanges(resource_commit, m_beginRenderingInfo.queue);

	m_pRenderContext->bindShader(m_pShader->getId(), m_beginRenderingInfo.queue);

	// Here i can draw

	m_pRenderContext->endRendering(m_beginRenderingInfo);
}