#include "BasePass.h"

#include <stdexcept>

using namespace engine::render;

void IBasePass::execute(f32 dt)
{
	if (m_bOneTime && m_bCanExecute)
		m_bCanExecute = false;
}

bool IBasePass::canExecute()
{
	return m_bCanExecute;
}

void IRenderPass::init(const FRenderPassCreateInfo& pass_ci)
{
	m_queue = pass_ci.queue;
	m_sizePolicy = pass_ci.sizePolicy;
	m_sizeTarget = pass_ci.sizeTarget;
	m_renderResources = pass_ci.resources;
	m_bakedBindings = pass_ci.bindings;
	m_bOneTime = pass_ci.oneTime;

	m_bFlipViewport = pass_ci.flipViewport;
	m_viewportIndex = pass_ci.viewportIndex;
	m_depthTarget = pass_ci.depthTarget;
	m_stencilTarget = pass_ci.stencilTarget;
}

void IRenderPass::build(const FPassBuildInfo& build_info)
{
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto& backbuffer_target = m_renderResources[ERenderResourceGroupType::eBackbufferWrite];
	for (u64 resource_id : backbuffer_target)
	{
		auto& color_attachment = m_vColorAttachments.emplace_back();
		buildAttachment(resource_id, build_info, color_attachment);
		color_attachment.backbuffer = true;
	}

	auto& output_targets = m_renderResources[ERenderResourceGroupType::eColorWrite];
	for (u64 resource_id : output_targets)
	{
		if (resource_id == m_depthTarget || resource_id == m_stencilTarget)
			continue;

		auto& color_attachment = m_vColorAttachments.emplace_back();
		buildAttachment(resource_id, build_info, color_attachment);
	}

	if (m_depthTarget != ~0ull)
		buildAttachment(m_depthTarget, build_info, m_depthAttachment);

	if (m_stencilTarget != ~0ull)
		buildAttachment(m_stencilTarget, build_info, m_stencilAttachment);
}

void IRenderPass::execute(f32 dt)
{
	IBasePass::execute(dt);

	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto& viewport = render_graph->getViewport(m_viewportIndex);
	auto& viewport_view = render_graph->getViewportView(m_viewportIndex);

	FBeginRendering2Info beginRenderingInfo{};
	beginRenderingInfo.queue = m_queue;
	beginRenderingInfo.colorAttachments = m_vColorAttachments;
	beginRenderingInfo.depthAttachment = m_depthAttachment;
	beginRenderingInfo.stencilAttachment = m_stencilAttachment;
	beginRenderingInfo.layerCount = m_layerCount;
	beginRenderingInfo.renderArea.size.x = viewport.size.x;
	beginRenderingInfo.renderArea.size.y = viewport.size.y;

#ifndef BUILD_WITHOUT_DEBUGGIND
	beginRenderingInfo.name = m_debugName;
#endif

	render_context->beginTimestampZone(m_queue);
	render_context->beginRendering(beginRenderingInfo);
	render_context->setViewport(utl::cast<f32>(beginRenderingInfo.renderArea.size.x), utl::cast<f32>(beginRenderingInfo.renderArea.size.y), m_bFlipViewport);

	FRenderFrameInfo frame_info{};
	frame_info.deltaTime = dt;
	frame_info.renderRect = beginRenderingInfo.renderArea;

	_render(frame_info);

	render_context->endRendering(beginRenderingInfo.queue);
	render_context->endTimestampZone(m_queue);
}

void IRenderPass::buildAttachment(u64 resource_id, const FPassBuildInfo& build_info, FRenderGraphAttachmentInfo& attachment_info)
{
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto& render_resource = render_graph->getRenderResource(resource_id);
	auto& resource_info = build_info.getAttachmentInfo(resource_id);

	attachment_info.handle = render_resource.handle;
	attachment_info.loadOp = resource_info.loadOp;
	attachment_info.storeOp = resource_info.storeOp;
}




void IComputePass::init(const FComputePassCreateInfo& pass_ci)
{
	m_queue = pass_ci.queue;
	m_sizePolicy = pass_ci.sizePolicy;
	m_sizeTarget = pass_ci.sizeTarget;
	m_renderResources = pass_ci.resources;
	m_bakedBindings = pass_ci.bindings;
	m_bOneTime = pass_ci.oneTime;
}

void IComputePass::build(const FPassBuildInfo& build_info)
{

}

void IComputePass::execute(f32 dt)
{
	IBasePass::execute(dt);

	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	FComputeFrameInfo frame_info{};
	frame_info.deltaTime = dt;

	if (m_sizePolicy == ERenderingSizeType::eFromRenderTarget && m_sizeTarget != ~0ull)
	{
		auto& render_resource = render_graph->getRenderResource(m_sizeTarget);

		auto& resource_sizes = render_context->getTextureInfo(render_resource.handle);
		m_dispatchGroup.x = resource_sizes.size.width;
		m_dispatchGroup.y = resource_sizes.size.height;
	}

	render_context->beginTimestampZone(m_queue);

	_execute(frame_info);

	render_context->endTimestampZone(m_queue);
}