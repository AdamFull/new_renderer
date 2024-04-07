#include "PostProcessPass.h"

#include <engine/render/ShaderObject.h>

using namespace engine::render;

CPostProcessPass::CPostProcessPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CPostProcessPass::init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::init(pass_ci);

	auto& render_graph = m_pRenderInterface->getRenderGraph();

	FComputeShaderCreateInfo shader_ci{};

	// Make RHI shader object
	m_pShader = stl::make_unique<CShaderObject>(m_pRenderInterface);
	m_pShader->create(pass_ci.shader, shader_ci);

	initShaderResources(m_pShader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = stl::format("postprocess:{}", pass_ci.shader);
#endif
}

void CPostProcessPass::build(const FPassBuildInfo& build_info)
{
	IComputePass::build(build_info);
}

void CPostProcessPass::_execute(const FComputeFrameInfo& frame_info)
{
}