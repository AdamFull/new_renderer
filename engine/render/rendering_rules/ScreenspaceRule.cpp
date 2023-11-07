#include "ScreenspaceRule.h"

using namespace engine::render;

CScreenspaceRule::CScreenspaceRule(IRenderInterface* render_interface, IRenderHandleInterface* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CScreenspaceRule::create(const FRenderingCreateInfo& create_info)
{
	m_renderingInfo.colorAttachments = create_info.renderTargets;
	m_renderingInfo.depthAttachment = create_info.depthTarget;
	m_renderingInfo.stencilAttachment = create_info.stencilTarget;
	m_renderingInfo.queue = create_info.queue;
	m_renderingInfo.flipViewport = create_info.flipViewport;

	FGraphicsShaderCreateInfo shader_ci{};
	shader_ci.colorAttachments = create_info.renderTargets;
	shader_ci.depthAttachment = create_info.depthTarget;
	shader_ci.stencilAttachment = create_info.stencilTarget;
	shader_ci.cullMode = SHADER_CULL_MODE_FRONT_BIT;
	shader_ci.frontFace = SHADER_FRONT_FACE_CCW;
	shader_ci.primitiveTopology = SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	auto shader_view = m_pRenderInterface->createShader(create_info.shader, shader_ci);
	auto shader_reflection = m_pRenderContext->getShaderReflection(shader_view);

	m_pShader = std::make_unique<CShaderObject>();
	m_pShader->create(shader_view, shader_reflection);
}

void CScreenspaceRule::render(float dt)
{
	// begin rendering
	// set viewport
	// set scissor
	// update shader uniforms and textures
	// submit shader
	// render
	// end rendering
}