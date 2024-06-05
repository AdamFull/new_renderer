#include "FullscreenPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CFullscreenPass::CFullscreenPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CFullscreenPass::Init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::Init(pass_ci);

	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	// Make RHI shader object
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface);

	auto cull_mode = pass_ci.flipViewport ? EShaderCullModeFlagBits::eBack : EShaderCullModeFlagBits::eFront;

	MakeShader(pass_ci.shader, m_pShader, cull_mode, EShaderFrontFace::eCCW);
	InitShaderResources(m_pShader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("fullscreen:{}", pass_ci.shader);
#endif
}

void CFullscreenPass::Build(const FPassBuildInfo& build_info)
{
	IRenderPass::Build(build_info);
}

void CFullscreenPass::_Render(const FRenderFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();

	BindShaderResources(m_pShader, frame_info.deltaTime);

	auto resolution = render_context->GetScreenRenderRect();
	render_context->SetViewport(resolution.size.x, resolution.size.y, m_bFlipViewport);
	render_context->SetScissorRect(0, 0, resolution.size.x, resolution.size.y);

	m_pShader->FlushDescriptors();
	m_pShader->FlushConstants(m_queue);
	m_pShader->Bind(m_queue);
	m_pShader->BindResources(m_queue);

	FGeometryDrawInfo draw_info{};
	auto& range = draw_info.ranges.emplace_back();
	range.beginIndex = 0ull;
	range.indexCount = 0ull;
	range.beginVertex = 0ull;
	range.vertexCount = 3ull;

	render_context->Draw(m_queue, draw_info);
}