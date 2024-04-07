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

void IBasePass::initShaderResources(const std::unique_ptr<IShaderObject>& shader)
{
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	// Bind default textures/buffers to shader
	for (const auto& [bind_point, resource_id] : m_bakedBindings)
	{
		auto& render_resource = render_graph->getRenderResource(resource_id);
		if (render_resource.type & ERenderResourceTypeFlagBits::eAllImage)
			shader->bindTexture(bind_point.c_str(), render_resource.handle);
		else if (render_resource.type == ERenderResourceTypeFlagBits::eBuffer)
			shader->bindBuffer(bind_point.c_str(), render_resource.handle);
	}
}

void IBasePass::bindShaderResources(const std::unique_ptr<IShaderObject>& shader, f32 dt)
{
	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& post_process = render_world->getPostProcessingData();

	if (auto& uniform = shader->getUniformBlock("FPostProcessEffects"); uniform)
	{
		uniform->set("enable_tonemap", static_cast<int32_t>(post_process.tonemap.enable));
		uniform->set("tm_gamma", post_process.tonemap.gamma);
		uniform->set("tm_exposure", post_process.tonemap.exposure);
		uniform->set("tm_white_point", post_process.tonemap.whitePoint);
		uniform->set("tm_adaptive", static_cast<int32_t>(post_process.tonemap.adaptive));

		// bloom uniforms
		uniform->set("enable_bloom", static_cast<int32_t>(post_process.bloom.enable));
		uniform->set("b_bloom_strength", post_process.bloom.strength);

		// Depth of field uniforms
		uniform->set("enable_dof", static_cast<int32_t>(post_process.dof.enable));

		uniform->set("df_focus_point", post_process.dof.focus_point);
		uniform->set("df_near_field", post_process.dof.near_field);
		uniform->set("df_near_transition", post_process.dof.near_transition);
		uniform->set("df_far_field", post_process.dof.far_field);
		uniform->set("df_far_transition", post_process.dof.far_transition);

		// Chromatic aberration uniforms
		uniform->set("enable_chromatic_aberration", static_cast<int32_t>(post_process.aberration.enable));
		uniform->set("ca_distortion", post_process.aberration.distortion);
		uniform->set("ca_iterations", post_process.aberration.iterations);

		// Finm grain uniforms
		uniform->set("enable_film_grain", static_cast<int32_t>(post_process.filmgrain.enable));
		uniform->set("fg_amount", post_process.filmgrain.amount);

		// Vignette uniforms
		uniform->set("enable_vignette", static_cast<int32_t>(post_process.vignette.enable));
		uniform->set("v_inner_radius", post_process.vignette.inner);
		uniform->set("v_outer_radius", post_process.vignette.outer);
		uniform->set("v_opacity", post_process.vignette.opacity);
	}
}

void IRenderPass::init(const FRenderPassCreateInfo& pass_ci)
{
	m_queue = pass_ci.queue;
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

	if (m_depthTarget != INVALID_HANDLE)
		buildAttachment(m_depthTarget, build_info, m_depthAttachment);

	if (m_stencilTarget != INVALID_HANDLE)
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
	beginRenderingInfo.colorAttachments = &m_vColorAttachments;
	beginRenderingInfo.depthAttachment = &m_depthAttachment;
	beginRenderingInfo.stencilAttachment = &m_stencilAttachment;
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

void IRenderPass::makeShader(const std::string& name, std::unique_ptr<IShaderObject>& shader, EShaderCullModeFlagBits cull_mode, EShaderFrontFace front_face, const IVertexHandler* vbo)
{
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	FGraphicsShaderCreateInfo shader_ci{};
	shader_ci.cullMode = cull_mode;
	shader_ci.frontFace = front_face;
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

	if (m_depthTarget != INVALID_HANDLE)
	{
		auto& render_resource = render_graph->getRenderResource(m_depthTarget);
		shader_ci.depthAttachment.source = render_resource.handle;
	}

	if (m_stencilTarget != INVALID_HANDLE)
	{
		auto& render_resource = render_graph->getRenderResource(m_stencilTarget);
		shader_ci.stencilAttachment.source = render_resource.handle;
	}

	if (vbo)
	{
		auto& render_packet = m_pRenderInterface->getRenderPacket();

		auto vbo_hash = vbo->getHash();
		shader_ci.usageCount = render_packet->stats.perTypeVBOCount[vbo_hash];
		shader_ci.vertexDeclaration = vbo->getVertexDeclaration();
	}

	shader->create(name, shader_ci);
}

void IRenderPass::initShaderResources(const std::unique_ptr<IShaderObject>& shader)
{
	IBasePass::initShaderResources(shader);
}

void IRenderPass::bindShaderResources(const std::unique_ptr<IShaderObject>& shader, f32 dt)
{
	IBasePass::bindShaderResources(shader, dt);
}



void IComputePass::init(const FComputePassCreateInfo& pass_ci)
{
	m_queue = pass_ci.queue;
	m_vDispatchInfos = pass_ci.groupInfos;
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

	if (m_vDispatchGroups.size() != m_vDispatchInfos.size())
		m_vDispatchGroups.resize(m_vDispatchInfos.size());

	for (u64 idx = 0ull; idx < m_vDispatchInfos.size(); ++idx)
	{
		const auto& group_info = m_vDispatchInfos[idx];
		auto& dispatch_info = m_vDispatchGroups[idx];

		if (group_info.type == ERenderingSizeType::eFromRenderTarget)
		{
			if (group_info.target != INVALID_HANDLE)
			{
				auto& render_resource = render_graph->getRenderResource(group_info.target);

				auto& resource_sizes = render_context->getTextureInfo(render_resource.handle);
				dispatch_info.globalSizes.x = resource_sizes.size.width;
				dispatch_info.globalSizes.y = resource_sizes.size.height;
				dispatch_info.globalSizes.z = 1u;
			}
		}
		else if (group_info.type == ERenderingSizeType::eManual)
		{
			dispatch_info.globalSizes.x = group_info.width;
			dispatch_info.globalSizes.y = group_info.height;
			dispatch_info.globalSizes.z = group_info.depth;
		}

		dispatch_info.divideByLocals = group_info.divideByLocals;
	}

	render_context->beginTimestampZone(m_queue);

	_execute(frame_info);

	render_context->endTimestampZone(m_queue);
}

void IComputePass::initShaderResources(const std::unique_ptr<IShaderObject>& shader)
{
	IBasePass::initShaderResources(shader);
}

void IComputePass::bindShaderResources(const std::unique_ptr<IShaderObject>& shader, f32 dt)
{
	IBasePass::bindShaderResources(shader, dt);

	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& post_process = render_world->getPostProcessingData();

	// For histogram calculation
	if (auto& uniform = shader->getUniformBlock("FHistogramComputeParams"); uniform)
	{
		float log_lum_range = post_process.tonemap.lumMax - post_process.tonemap.lumMin;
		float time_coeff = glm::clamp<float>(1.0f - glm::exp(-dt * post_process.tonemap.tau), 0.0, 1.0);

		// TODO: it's incorrect now. I should get size from input target
		auto render_size = render_context->getRenderResolution();

		uniform->set("minLogLum", post_process.tonemap.lumMin);
		uniform->set("logLumRange", log_lum_range);
		uniform->set("timeCoeff", time_coeff);
		uniform->set("numPixelsX", utl::cast<f32>(render_size.x));
		uniform->set("numPixelsY", utl::cast<f32>(render_size.y));
	}
}