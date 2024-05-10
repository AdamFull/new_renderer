#include "FullscreenPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CFullscreenPass::CFullscreenPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CFullscreenPass::init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::init(pass_ci);

	auto& render_graph = m_pRenderInterface->getRenderGraph();

	// Make RHI shader object
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface);

	auto cull_mode = pass_ci.flipViewport ? EShaderCullModeFlagBits::eBack : EShaderCullModeFlagBits::eFront;

	makeShader(pass_ci.shader, m_pShader, cull_mode, EShaderFrontFace::eCCW);
	initShaderResources(m_pShader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("fullscreen:{}", pass_ci.shader);
#endif
}

void CFullscreenPass::build(const FPassBuildInfo& build_info)
{
	IRenderPass::build(build_info);
}

void CFullscreenPass::_render(const FRenderFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	bindShaderResources(m_pShader, frame_info.deltaTime);

	auto resolution = render_context->getScreenRenderRect();
	render_context->setViewport(resolution.size.x, resolution.size.y, m_bFlipViewport);
	render_context->setScissorRect(0, 0, resolution.size.x, resolution.size.y);

	m_pShader->flushDescriptors();
	m_pShader->flushConstants(m_queue);
	m_pShader->bind(m_queue);

	FGeometryDrawInfo draw_info{};
	auto& range = draw_info.ranges.emplace_back();
	range.beginIndex = 0ull;
	range.indexCount = 0ull;
	range.beginVertex = 0ull;
	range.vertexCount = 3ull;

	render_context->draw(m_queue, draw_info);
}