#include "ComputePass.h"

#include <engine/render/ShaderObject.h>

#include <format>

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
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface);
	m_pShader->create(pass_ci.shader, shader_ci);

	// Bind default textures/buffers to shader
	for (const auto& [bind_point, resource_id] : m_bakedBindings)
	{
		auto& render_resource = render_graph->getRenderResource(resource_id);
		if (render_resource.type & ERenderResourceTypeFlagBits::eAllImage)
			m_pShader->bindTexture(bind_point.c_str(), render_resource.handle);
		else if (render_resource.type == ERenderResourceTypeFlagBits::eBuffer)
			m_pShader->bindBuffer(bind_point.c_str(), render_resource.handle);
	}

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("compute:{}", pass_ci.shader);
#endif
}

void CComputePass::bake(std::unordered_map<u64, FRenderGraphAttachmentOperationInfo>& resource_info)
{
	IComputePass::bake(resource_info);
}

void CComputePass::_execute(const FComputeFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	m_pShader->flushDescriptors();
	m_pShader->flushConstants(m_queue);
	m_pShader->bind(m_queue);

	FDispatch2Info dispatch_info{};
	dispatch_info.globalSizes = m_dispatchGroup;
	dispatch_info.shaderHandle = m_pShader->handle();

	render_context->dispatch2(m_queue, dispatch_info);
}