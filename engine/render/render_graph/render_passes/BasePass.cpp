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

void IRenderPass::bake(std::unordered_map<size_t, FRenderGraphAttachmentOperationInfo>& resource_info)
{
	auto& output_targets = m_renderResources[RENDER_RESOURCE_GROUP_COLOR_WRITE];
	for (size_t resource_id : output_targets)
	{
		auto& render_resource = m_pRenderGraph->getRenderResource(resource_id);

		auto& color_attachment = m_vColorAttachments.emplace_back();
		color_attachment.handle = render_resource.handle;
		color_attachment.loadOp = resource_info[resource_id].loadOp;
		color_attachment.storeOp = resource_info[resource_id].storeOp;
	}

	if (m_depthTarget != ~0ull)
	{
		auto& render_resource = m_pRenderGraph->getRenderResource(m_depthTarget);
		m_depthAttachment.handle = render_resource.handle;
		m_depthAttachment.loadOp = resource_info[m_depthTarget].loadOp;
		m_depthAttachment.storeOp = resource_info[m_depthTarget].storeOp;
	}

	if (m_stencilTarget != ~0ull)
	{
		auto& render_resource = m_pRenderGraph->getRenderResource(m_stencilTarget);
		m_stencilAttachment.handle = render_resource.handle;
		m_stencilAttachment.loadOp = resource_info[m_stencilTarget].loadOp;
		m_stencilAttachment.storeOp = resource_info[m_stencilTarget].storeOp;
	}
}

void IRenderPass::execute(float dt)
{
	FBeginRendering2Info beginRenderingInfo{};
	beginRenderingInfo.queue = m_queue;
	beginRenderingInfo.flipViewport = m_bFlipViewport;
	beginRenderingInfo.colorAttachments = m_vColorAttachments;
	beginRenderingInfo.depthAttachment = m_depthAttachment;
	beginRenderingInfo.stencilAttachment = m_stencilAttachment;
	beginRenderingInfo.layerCount = m_layerCount;

	if (m_sizePolicy == RENDER_GRAPH_SIZE_POLICY_FROM_TARGET && m_sizeTarget != ~0ull)
	{
		auto& render_resource = m_pRenderGraph->getRenderResource(m_sizeTarget);
		if (render_resource.type != RENDER_RESOURCE_TYPE_RENDER_TARGET)
		{
			throw std::runtime_error("Incorrect render resource type.");
		}

		auto& resource_sizes = m_pRenderContext->getTextureInfo(render_resource.handle);
		beginRenderingInfo.renderArea.size.x = resource_sizes.width;
		beginRenderingInfo.renderArea.size.y = resource_sizes.height;
	}

#ifndef BUILD_WITHOUT_DEBUGGIND
	beginRenderingInfo.name = m_debugName;
#endif

	m_pRenderContext->beginRendering2(beginRenderingInfo);
	m_pRenderContext->setViewport(static_cast<float>(beginRenderingInfo.renderArea.size.x), static_cast<float>(beginRenderingInfo.renderArea.size.y), beginRenderingInfo.flipViewport);

	FRenderFrameInfo frame_info{};
	frame_info.deltaTime = dt;
	frame_info.renderRect = beginRenderingInfo.renderArea;

	_render(frame_info);

	m_pRenderContext->endRendering2(beginRenderingInfo.queue);
}




void IComputePass::init(const FComputePassCreateInfo& pass_ci)
{
	m_queue = pass_ci.queue;
	m_sizePolicy = pass_ci.sizePolicy;
	m_sizeTarget = pass_ci.sizeTarget;
	m_renderResources = pass_ci.resources;
	m_bakedBindings = pass_ci.bindings;
}

void IComputePass::bake(std::unordered_map<size_t, FRenderGraphAttachmentOperationInfo>& resource_info)
{

}

void IComputePass::execute(float dt)
{
	FComputeFrameInfo frame_info{};
	frame_info.deltaTime = dt;

	_execute(frame_info);
}