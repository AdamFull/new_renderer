#include "ComputePass.h"

using namespace engine::render;

CComputePass::CComputePass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputePass::Init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::Init(pass_ci);

	auto& resource_manager = m_pRenderInterface->GetResourceManager();

	FComputeShaderCreateInfo shader_ci{};

	// Make RHI shader object
	m_shader = resource_manager->CreateShader(pass_ci.shader, shader_ci);
	InitShaderResources(m_shader);

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
	auto& resource_manager = m_pRenderInterface->GetResourceManager();
	auto& render_context = m_pRenderInterface->GetRenderContext();

	auto& shader = resource_manager->GetShaderObject(m_shader);

	BindShaderResources(m_shader, frame_info.deltaTime);

	shader->FlushDescriptors();
	shader->FlushConstants(m_queue);
	shader->Bind(m_queue);
	shader->BindResources(m_queue);

	for (u64 dispatch_idx = 0ull; dispatch_idx < m_vDispatchGroups.size(); ++dispatch_idx)
	{
		auto& dispatch_info = m_vDispatchGroups[dispatch_idx];
		dispatch_info.shaderHandle = shader->Handle();

		render_context->Dispatch(m_queue, dispatch_info, dispatch_idx);

		if (m_vDispatchGroups.size() > 0 && dispatch_idx < m_vDispatchGroups.size() - 1)
			render_context->PushC2CBarrier(m_queue);
	}
}