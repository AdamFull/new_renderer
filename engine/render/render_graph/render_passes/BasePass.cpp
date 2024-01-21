#include "BasePass.h"

#include <stdexcept>

using namespace engine::render;

void IRenderPass::init(const FRenderPassCreateInfo& pass_ci)
{
	m_queue = pass_ci.queue;
	m_sizePolicy = pass_ci.sizePolicy;
	m_sizeTarget = pass_ci.sizeTarget;
	m_renderResources = pass_ci.resources;
	m_bakedBindings = pass_ci.bindings;

	m_bFlipViewport = pass_ci.flipViewport;
	m_depthTarget = pass_ci.depthTarget;
	m_stencilTarget = pass_ci.stencilTarget;
}

void IRenderPass::build(const FPassBuildInfo& build_info)
{
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto& output_targets = m_renderResources[ERenderResourceGroupType::eColorWrite];
	for (u64 resource_id : output_targets)
	{
		if (resource_id == m_depthTarget || resource_id == m_stencilTarget)
			continue;

		auto& render_resource = render_graph->getRenderResource(resource_id);
		auto& resource_info = build_info.getAttachmentInfo(resource_id);

		auto& color_attachment = m_vColorAttachments.emplace_back();
		color_attachment.handle = render_resource.handle;
		color_attachment.loadOp = resource_info.loadOp;
		color_attachment.storeOp = resource_info.storeOp;
	}

	if (m_depthTarget != ~0ull)
	{
		auto& render_resource = render_graph->getRenderResource(m_depthTarget);
		auto& resource_info = build_info.getAttachmentInfo(m_depthTarget);

		m_depthAttachment.handle = render_resource.handle;
		m_depthAttachment.loadOp = resource_info.loadOp;
		m_depthAttachment.storeOp = resource_info.storeOp;
	}

	if (m_stencilTarget != ~0ull)
	{
		auto& render_resource = render_graph->getRenderResource(m_stencilTarget);
		auto& resource_info = build_info.getAttachmentInfo(m_stencilTarget);

		m_stencilAttachment.handle = render_resource.handle;
		m_stencilAttachment.loadOp = resource_info.loadOp;
		m_stencilAttachment.storeOp = resource_info.storeOp;
	}
}

void IRenderPass::execute(f32 dt)
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	FBeginRendering2Info beginRenderingInfo{};
	beginRenderingInfo.queue = m_queue;
	beginRenderingInfo.flipViewport = m_bFlipViewport;
	beginRenderingInfo.colorAttachments = m_vColorAttachments;
	beginRenderingInfo.depthAttachment = m_depthAttachment;
	beginRenderingInfo.stencilAttachment = m_stencilAttachment;
	beginRenderingInfo.layerCount = m_layerCount;

	if (m_sizePolicy == ERenderingSizeType::eFromRenderTarget && m_sizeTarget != ~0ull)
	{
		auto& render_resource = render_graph->getRenderResource(m_sizeTarget);
		if (render_resource.type != ERenderResourceTypeFlagBits::eRenderTarget)
		{
			throw std::runtime_error("Incorrect render resource type.");
		}

		auto& resource_sizes = render_context->getTextureInfo(render_resource.handle);
		beginRenderingInfo.renderArea.size.x = resource_sizes.width;
		beginRenderingInfo.renderArea.size.y = resource_sizes.height;
	}

#ifndef BUILD_WITHOUT_DEBUGGIND
	beginRenderingInfo.name = m_debugName;
#endif

	render_context->beginRendering2(beginRenderingInfo);
	render_context->setViewport(utl::cast<f32>(beginRenderingInfo.renderArea.size.x), utl::cast<f32>(beginRenderingInfo.renderArea.size.y), beginRenderingInfo.flipViewport);

	FRenderFrameInfo frame_info{};
	frame_info.deltaTime = dt;
	frame_info.renderRect = beginRenderingInfo.renderArea;

	_render(frame_info);

	render_context->endRendering2(beginRenderingInfo.queue);
}




void IComputePass::init(const FComputePassCreateInfo& pass_ci)
{
	m_queue = pass_ci.queue;
	m_sizePolicy = pass_ci.sizePolicy;
	m_sizeTarget = pass_ci.sizeTarget;
	m_renderResources = pass_ci.resources;
	m_bakedBindings = pass_ci.bindings;
}

void IComputePass::build(const FPassBuildInfo& build_info)
{

}

void IComputePass::execute(f32 dt)
{
	FComputeFrameInfo frame_info{};
	frame_info.deltaTime = dt;

	_execute(frame_info);
}