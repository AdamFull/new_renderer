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

	// Create shader
	FGraphicsShaderCreateInfo shader_ci{};
	shader_ci.cullMode = pass_ci.flipViewport ? EShaderCullModeFlagBits::eBack : EShaderCullModeFlagBits::eFront;
	shader_ci.frontFace = EShaderFrontFace::eCCW;
	shader_ci.primitiveTopology = EVertexPrimitiveTopology::eTriangleList;

	auto& output_targets = m_renderResources[ERenderResourceGroupType::eColorWrite];
	for (u64 resource_id : output_targets)
	{
		if (resource_id == m_depthTarget || resource_id == m_stencilTarget)
			continue;

		auto& render_resource = render_graph->getRenderResource(resource_id);
		auto& new_attachment = shader_ci.colorAttachments.emplace_back();
		new_attachment.source = render_resource.handle;
	}

	auto& backbuffer_targer = m_renderResources[ERenderResourceGroupType::eBackbufferWrite];
	for (u64 resource_id : backbuffer_targer)
	{
		if (resource_id == m_depthTarget || resource_id == m_stencilTarget)
			continue;

		auto& render_resource = render_graph->getRenderResource(resource_id);
		auto& new_attachment = shader_ci.colorAttachments.emplace_back();
		new_attachment.source = render_resource.handle;
		new_attachment.backbuffer = true;
	}

	if (m_depthTarget != ~0ull)
	{
		auto& render_resource = render_graph->getRenderResource(m_depthTarget);
		shader_ci.depthAttachment.source = render_resource.handle;
	}

	if (m_stencilTarget != ~0ull)
	{
		auto& render_resource = render_graph->getRenderResource(m_stencilTarget);
		shader_ci.stencilAttachment.source = render_resource.handle;
	}

	// Make RHI shader object
	m_pShader = stl::make_unique<CShaderObject>(m_pRenderInterface);
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
	m_debugName = stl::format("fullscreen:{}", pass_ci.shader);
#endif
}

void CFullscreenPass::build(const FPassBuildInfo& build_info)
{
	IRenderPass::build(build_info);
}

void CFullscreenPass::_render(const FRenderFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->setScissorRect(0, 0, frame_info.renderRect.size.x, frame_info.renderRect.size.y);

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