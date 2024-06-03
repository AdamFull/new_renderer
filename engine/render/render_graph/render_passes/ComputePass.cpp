#include "ComputePass.h"

#include <engine/render/ShaderObject.h>

using namespace engine::render;

CComputePass::CComputePass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputePass::Init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::Init(pass_ci);

	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	FComputeShaderCreateInfo shader_ci{};

	// Make RHI shader object
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface);
	m_pShader->Create(pass_ci.shader, shader_ci);

	InitShaderResources(m_pShader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("compute:{}", pass_ci.shader);
#endif
}

void CComputePass::Build(const FPassBuildInfo& build_info)
{
	IComputePass::Build(build_info);
}

void CComputePass::_Execute(const FComputeFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();

	BindShaderResources(m_pShader, frame_info.deltaTime);

	m_pShader->FlushDescriptors();
	m_pShader->FlushConstants(m_queue);
	m_pShader->Bind(m_queue);

	for (u64 dispatch_idx = 0ull; dispatch_idx < m_vDispatchGroups.size(); ++dispatch_idx)
	{
		auto& dispatch_info = m_vDispatchGroups[dispatch_idx];
		dispatch_info.shaderHandle = m_pShader->Handle();

		render_context->Dispatch(m_queue, dispatch_info, dispatch_idx);

		if (m_vDispatchGroups.size() > 0 && dispatch_idx < m_vDispatchGroups.size() - 1)
			render_context->PushC2CBarrier(m_queue);
	}
}