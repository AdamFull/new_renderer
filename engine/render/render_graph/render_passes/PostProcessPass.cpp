#include "PostProcessPass.h"

#include <engine/render/ShaderObject.h>

using namespace engine::render;

CPostProcessPass::CPostProcessPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CPostProcessPass::Init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::Init(pass_ci);

	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	FComputeShaderCreateInfo shader_ci{};

	// Make RHI shader object
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface);
	m_pShader->Create(pass_ci.shader, shader_ci);

	InitShaderResources(m_pShader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("postprocess:{}", pass_ci.shader);
#endif
}

void CPostProcessPass::Build(const FPassBuildInfo& build_info)
{
	IComputePass::Build(build_info);
}

void CPostProcessPass::_Execute(const FComputeFrameInfo& frame_info)
{
}