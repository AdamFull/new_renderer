#include "FullscreenRenderingStep.h"

#include <engine/render/ShaderObject.h>
#include <logger/logger.h>

#include <glm/glm.hpp>

#include <engine/profiler/Profiler.h>

using namespace engine::render;

CFullscreenRenderingStep::CFullscreenRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CFullscreenRenderingStep::_create(const FRenderingStepCreateInfo& create_info)
{
	TRACY_PROFILE_FUNCTION();

	auto& rendering_ci = create_info.rendering;

	if (rendering_ci.shaderName.empty())
		log_error("No shader name provided for fullscreen rendering step creation.");

	// Create shader
	FGraphicsShaderCreateInfo shader_ci{};
	shader_ci.cullMode = rendering_ci.flipViewport ? SHADER_CULL_MODE_BACK_BIT : SHADER_CULL_MODE_FRONT_BIT;
	shader_ci.frontFace = SHADER_FRONT_FACE_CCW;
	shader_ci.primitiveTopology = SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	shader_ci.colorAttachments = rendering_ci.renderTargets;
	shader_ci.depthAttachment = rendering_ci.depthTarget;
	shader_ci.stencilAttachment = rendering_ci.stencilTarget;

	// Make RHI shader object
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface, m_pRenderContext);
	m_pShader->create(rendering_ci.shaderName, shader_ci);

	// Bind default textures to shader
	for (auto& [binding_point, texture] : rendering_ci.shaderInputs)
		m_pShader->bindTexture(binding_point.c_str(), texture);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_beginRenderingInfo.name = std::format("fullscreen:{}", rendering_ci.shaderName);
#endif
}

void CFullscreenRenderingStep::_execute(float dt)
{
	TRACY_PROFILE_FUNCTION();

	m_pRenderContext->beginRendering(m_beginRenderingInfo);

	auto& rect = m_beginRenderingInfo.renderArea;
	m_pRenderContext->setViewport(static_cast<float>(rect.size.x), static_cast<float>(rect.size.y), m_beginRenderingInfo.flipViewport);
	m_pRenderContext->setScissorRect(0, 0, rect.size.x, rect.size.y);

	//auto& push_block = m_pShader->getUniformBlock("FUniformData");
	//push_block->set("color", glm::vec3(2.f, 2.f, 2.f));

	//m_pShader->bindTexture("penis", 1);

	// Here i can update uniforms

	// Update all shader uniforms and other things. It should be before i bind shader
	m_pShader->flushDescriptors();
	m_pShader->flushConstants(m_beginRenderingInfo.queue);
	m_pShader->bind(m_beginRenderingInfo.queue);

	FGeometryDrawInfo draw_info{};
	draw_info.ranges.emplace_back(FGeometryRanges{ 0ull, 0ull, 0ull, 3ull });

	m_pRenderContext->draw(m_beginRenderingInfo.queue, draw_info);

	// Here i can draw

	m_pRenderContext->endRendering(m_beginRenderingInfo);
}