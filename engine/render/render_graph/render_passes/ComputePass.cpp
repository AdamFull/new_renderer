#include "ComputePass.h"

#include <engine/render/ShaderObject.h>

using namespace engine::render;

CComputePass::CComputePass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputePass::init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::init(pass_ci);

	auto& render_graph = m_pRenderInterface->getRenderGraph();

	FComputeShaderCreateInfo shader_ci{};

	// Make RHI shader object
	m_pShader = stl::make_unique<CShaderObject>(m_pRenderInterface);
	m_pShader->create(pass_ci.shader, shader_ci);

	initShaderResources(m_pShader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = stl::format("compute:{}", pass_ci.shader);
#endif
}

void CComputePass::build(const FPassBuildInfo& build_info)
{
	IComputePass::build(build_info);
}

void CComputePass::_execute(const FComputeFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	bindShaderResources(m_pShader, frame_info.deltaTime);

	m_pShader->flushDescriptors();
	m_pShader->flushConstants(m_queue);
	m_pShader->bind(m_queue);

	for (u64 dispatch_idx = 0ull; dispatch_idx < m_vDispatchGroups.size(); ++dispatch_idx)
	{
		auto& dispatch_info = m_vDispatchGroups[dispatch_idx];
		dispatch_info.shaderHandle = m_pShader->handle();

		render_context->dispatch(m_queue, dispatch_info, dispatch_idx);

		if (m_vDispatchGroups.size() > 0 && dispatch_idx < m_vDispatchGroups.size() - 1)
			render_context->pushC2CBarrier(m_queue);
	}
}