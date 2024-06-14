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
	m_shader = m_pRenderInterface->CreateShaderObject(pass_ci.shader, shader_ci);

	InitShaderResources(m_shader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("compute:{}", pass_ci.shader);
#endif
}

void CComputePass::Build(const FPassBuildInfo& build_info)
{
	IComputePass::Build(build_info);
}

void CComputePass::Update(f32 dt)
{
	m_deltaTime = dt;
}

void CComputePass::_Execute()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& render_graph = m_pRenderInterface->GetRenderGraph();
	auto& shader = m_pRenderInterface->GetShaderObject(m_shader);

	BindShaderResources(m_shader, m_deltaTime);

	shader->FlushDescriptors();
	shader->FlushConstants(m_queue);
	shader->Bind(m_queue);
	shader->BindResources(m_queue);

	auto resource_key = packResourceGroup(ERenderResourceAccessFlagBits::eRead, ERenderResourceGroupFlagBits::eIndirectArgument);
	if (auto found = m_renderResources.find(resource_key); found != m_renderResources.end())
	{
		// It can be only one argiment buffer
		auto& resource = found->second.front();
		auto& render_resource = render_graph->GetRenderResource(resource.resource);

		// We can draw indirect
		FDispatchIndirectInfo command{};
		command.argumentBuffer = render_resource.handle;
		command.argumentOffset = resource.subresource;
		render_context->Dispatch(m_queue, command);
	}
	else
	{
		m_dispatchGroup.shaderHandle = shader->Handle();
		render_context->Dispatch(m_queue, m_dispatchGroup, 0);
	}
}