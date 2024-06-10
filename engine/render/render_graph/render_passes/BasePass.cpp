#include "BasePass.h"

#include <stdexcept>

using namespace engine::render;

void caclulate_size_for_mip_level(u32& width, u32& height, u32 mip_level)
{
	if (mip_level == ~0u)
		mip_level = 0u;

	width = std::max(1u, width >> mip_level);
	height = std::max(1u, height >> mip_level);
}

void IBasePass::Execute(f32 dt)
{
	if (m_bOneTime && m_bCanExecute)
		m_bCanExecute = false;
}

bool IBasePass::CanExecute()
{
	return m_bCanExecute;
}

void IBasePass::InitShaderResources(FRHIHandle shader)
{
	auto& shader_object = m_pRenderInterface->GetShaderObject(shader);
	InitShaderResources(shader_object);
}

void IBasePass::BindShaderResources(FRHIHandle shader, f32 dt)
{

}

void IBasePass::InitShaderResources(const std::unique_ptr<IShaderObject>& shader)
{
	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	// Bind default textures/buffers to shader
	for (const auto& [bind_point, resource_binding] : m_bakedBindings)
	{
		auto& render_resource = render_graph->GetRenderResource(resource_binding.resource);
		if (render_resource.type & ERenderResourceTypeFlagBits::eAllImage)
			shader->BindTexture(utl::fnv1a_64_hash_cstr(bind_point.c_str()), render_resource.handle, resource_binding.subresource);
		else if (render_resource.type == ERenderResourceTypeFlagBits::eBuffer)
			shader->BindBuffer(utl::fnv1a_64_hash_cstr(bind_point.c_str()), render_resource.handle);
	}
}

void IBasePass::BindShaderResources(const std::unique_ptr<IShaderObject>& shader, f32 dt)
{

}

void IRenderPass::Init(const FRenderPassCreateInfo& pass_ci)
{
	m_queue = pass_ci.queue;
	m_renderResources = pass_ci.resources;
	m_bakedBindings = pass_ci.bindings;
	m_bOneTime = pass_ci.oneTime;

	m_clearValues = pass_ci.clearValues;
	m_bFlipViewport = pass_ci.flipViewport;
	m_depthTarget = pass_ci.depthTarget;
	m_stencilTarget = pass_ci.stencilTarget;
}

void IRenderPass::Build(const FPassBuildInfo& build_info)
{
	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	auto& backbuffer_target = m_renderResources[ERenderResourceGroupType::eBackbufferWrite];
	for (const auto& resource_binding : backbuffer_target)
	{
		auto& color_attachment = m_vColorAttachments.emplace_back();
		BuildAttachment(resource_binding.resource, build_info, color_attachment);
		color_attachment.backbuffer = true;
	}

	auto& output_targets = m_renderResources[ERenderResourceGroupType::eColorWrite];
	for (const auto& resource_binding : output_targets)
	{
		if (resource_binding.resource == m_depthTarget || resource_binding.resource == m_stencilTarget)
			continue;

		auto& color_attachment = m_vColorAttachments.emplace_back();
		BuildAttachment(resource_binding.resource, build_info, color_attachment);
	}

	if (m_depthTarget)
		BuildAttachment(m_depthTarget, build_info, m_depthAttachment);

	if (m_stencilTarget)
		BuildAttachment(m_stencilTarget, build_info, m_stencilAttachment);
}

void IRenderPass::Execute(f32 dt)
{
	IBasePass::Execute(dt);

	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	auto render_size = render_context->GetScreenRenderRect();

	FBeginRendering2Info beginRenderingInfo{};
	beginRenderingInfo.queue = m_queue;
	beginRenderingInfo.colorAttachments = &m_vColorAttachments;
	beginRenderingInfo.depthAttachment = &m_depthAttachment;
	beginRenderingInfo.stencilAttachment = &m_stencilAttachment;
	beginRenderingInfo.layerCount = m_layerCount;
	beginRenderingInfo.clearValues = m_clearValues;

	if (m_cameraIndex != ~0ull)
	{
		auto& camera = render_world->GetCamera(m_cameraIndex);
		beginRenderingInfo.renderArea.size.x = camera.size.x;
		beginRenderingInfo.renderArea.size.y = camera.size.y;
	}
	else
	{
		auto screen_rect = render_context->GetScreenRenderRect();
		beginRenderingInfo.renderArea.size.x = screen_rect.size.x;
		beginRenderingInfo.renderArea.size.y = screen_rect.size.y;
	}
	

#ifndef BUILD_WITHOUT_DEBUGGIND
	beginRenderingInfo.name = m_debugName;
#endif

	render_context->BeginTimestampZone(m_queue);

	if (CanRender())
	{
		render_context->BeginRendering(beginRenderingInfo);
		render_context->SetViewport(utl::cast<f32>(beginRenderingInfo.renderArea.size.x), utl::cast<f32>(beginRenderingInfo.renderArea.size.y), m_bFlipViewport);

		FRenderFrameInfo frame_info{};
		frame_info.deltaTime = dt;
		frame_info.renderRect = beginRenderingInfo.renderArea;

		_Render(frame_info);

		render_context->EndRendering(beginRenderingInfo.queue);
	}
	
	render_context->EndTimestampZone(m_queue);
}

void IRenderPass::BuildAttachment(FGraphHandle resource_id, const FPassBuildInfo& build_info, FRenderGraphAttachmentInfo& attachment_info)
{
	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	auto& render_resource = render_graph->GetRenderResource(resource_id);
	auto& resource_info = build_info.getAttachmentInfo(resource_id);

	attachment_info.handle = render_resource.handle;
	attachment_info.loadOp = resource_info.loadOp;
	attachment_info.storeOp = resource_info.storeOp;
}

FRHIHandle IRenderPass::MakeShader(const std::string& name, EShaderCullModeFlagBits cull_mode, EShaderFrontFace front_face, bool alphaBlend, EDepthCompareOp depthOp, const IVertexHandler* vbo)
{
	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	FGraphicsShaderCreateInfo shader_ci{};
	shader_ci.cullMode = cull_mode;
	shader_ci.frontFace = front_face;
	shader_ci.primitiveTopology = EVertexPrimitiveTopology::eTriangleList;
	shader_ci.depthCompareOp = depthOp;
	shader_ci.alphaBlending = alphaBlend;

	auto& output_targets = m_renderResources[ERenderResourceGroupType::eColorWrite];
	for (const auto& resource_binding : output_targets)
	{
		if (resource_binding.resource == m_depthTarget || resource_binding.resource == m_stencilTarget)
			continue;

		auto& render_resource = render_graph->GetRenderResource(resource_binding.resource);
		auto& new_attachment = shader_ci.colorAttachments.emplace_back();
		new_attachment.source = render_resource.handle;
	}

	auto& backbuffer_targer = m_renderResources[ERenderResourceGroupType::eBackbufferWrite];
	for (const auto& resource_binding : backbuffer_targer)
	{
		if (resource_binding.resource == m_depthTarget || resource_binding.resource == m_stencilTarget)
			continue;

		auto& render_resource = render_graph->GetRenderResource(resource_binding.resource);
		auto& new_attachment = shader_ci.colorAttachments.emplace_back();
		new_attachment.backbuffer = true;
	}

	if (m_depthTarget)
	{
		auto& render_resource = render_graph->GetRenderResource(m_depthTarget);
		shader_ci.depthAttachment.source = render_resource.handle;
	}

	if (m_stencilTarget)
	{
		auto& render_resource = render_graph->GetRenderResource(m_stencilTarget);
		shader_ci.stencilAttachment.source = render_resource.handle;
	}

	if (vbo)
	{
		auto& render_packet = m_pRenderInterface->GetRenderPacket();

		auto vbo_hash = vbo->GetHash();
		auto vbo_count = render_packet->stats.perTypeVBOCount[vbo_hash];

		
		shader_ci.usageCount = vbo_count ? vbo_count : 1u;
		shader_ci.vertexDeclaration = vbo->GetVertexDeclaration();
	}

	return m_pRenderInterface->CreateShaderObject(name, shader_ci);
}

void IRenderPass::InitShaderResources(FRHIHandle shader)
{
	IBasePass::InitShaderResources(shader);
}

void IRenderPass::BindShaderResources(FRHIHandle shader, f32 dt)
{
	IBasePass::BindShaderResources(shader, dt);
}

void IRenderPass::InitShaderResources(const std::unique_ptr<IShaderObject>& shader)
{
	IBasePass::InitShaderResources(shader);
}

void IRenderPass::BindShaderResources(const std::unique_ptr<IShaderObject>& shader, f32 dt)
{
	IBasePass::BindShaderResources(shader, dt);
}




void IComputePass::Init(const FComputePassCreateInfo& pass_ci)
{
	m_queue = pass_ci.queue;
	m_dispatchInfo = pass_ci.groupInfo;
	m_renderResources = pass_ci.resources;
	m_bakedBindings = pass_ci.bindings;
	m_bOneTime = pass_ci.oneTime;
}

void IComputePass::Build(const FPassBuildInfo& build_info)
{

}

void IComputePass::Execute(f32 dt)
{
	IBasePass::Execute(dt);

	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	FComputeFrameInfo frame_info{};
	frame_info.deltaTime = dt;

	if (m_dispatchInfo.type == ERenderingSizeType::eFromRenderTarget)
	{
		if (m_dispatchInfo.target)
		{
			auto resource = render_graph->GetRenderResource(m_dispatchInfo.target);
			auto texture_view = render_context->GetTextureView(resource.handle);
			m_dispatchGroup.globalSizes.x = texture_view.width;
			m_dispatchGroup.globalSizes.y = texture_view.height;
			m_dispatchGroup.globalSizes.z = 1u;
		}
	}
	else if (m_dispatchInfo.type == ERenderingSizeType::eFromBuffer)
	{
		auto resource = render_graph->GetRenderResource(m_dispatchInfo.target);
		auto buffer_view = render_context->GetBufferView(resource.handle);

		m_dispatchGroup.globalSizes.x = buffer_view.alignedSize / buffer_view.elementCount;
		m_dispatchGroup.globalSizes.y = 1u;
		m_dispatchGroup.globalSizes.z = 1u;
	}
	else if (m_dispatchInfo.type == ERenderingSizeType::eManual)
	{
		m_dispatchGroup.globalSizes.x = m_dispatchInfo.width;
		m_dispatchGroup.globalSizes.y = m_dispatchInfo.height;
		m_dispatchGroup.globalSizes.z = m_dispatchInfo.depth;
	}

	caclulate_size_for_mip_level(m_dispatchGroup.globalSizes.x, m_dispatchGroup.globalSizes.y, m_dispatchInfo.subresource);

	m_dispatchGroup.divideByLocals = m_dispatchInfo.divideByLocals;


	FBeginComputeInfo compute_begin_info{};
	compute_begin_info.queue = m_queue;

#ifndef BUILD_WITHOUT_DEBUGGIND
	compute_begin_info.name = m_debugName;
#endif

	render_context->BeginTimestampZone(m_queue);
	render_context->BeginCompute(compute_begin_info);

	_Execute(frame_info);

	render_context->EndCompute(m_queue);
	render_context->EndTimestampZone(m_queue);
}

void IComputePass::InitShaderResources(FRHIHandle shader)
{
	IBasePass::InitShaderResources(shader);
}

void IComputePass::BindShaderResources(FRHIHandle shader, f32 dt)
{
	auto& shader_object = m_pRenderInterface->GetShaderObject(shader);
	BindShaderResources(shader_object, dt);
}

void IComputePass::InitShaderResources(const std::unique_ptr<IShaderObject>& shader)
{
	IBasePass::InitShaderResources(shader);
}

void IComputePass::BindShaderResources(const std::unique_ptr<IShaderObject>& shader, f32 dt)
{
	IBasePass::BindShaderResources(shader, dt);

	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	auto& post_process = render_world->GetPostProcessingData();

	// For histogram calculation
	if (auto& uniform = shader->GetUniformBlock(shader_bindings::histogram_compute_params); uniform)
	{
		float log_lum_range = post_process.tonemap.lumMax - post_process.tonemap.lumMin;
		float time_coeff = glm::clamp<f32>(1.0f - glm::exp(-dt * post_process.tonemap.tau), 0.0, 1.0);

		// TODO: it's incorrect now. I should get size from input target
		auto render_size = render_context->GetRenderResolution();

		uniform->Set(shader_bindings::min_log_lum_label, post_process.tonemap.lumMin);
		uniform->Set(shader_bindings::log_lum_range_label, log_lum_range);
		uniform->Set(shader_bindings::time_coeff_label, time_coeff);
		uniform->Set(shader_bindings::time_num_pixels_x_label, utl::cast<f32>(render_size.x));
		uniform->Set(shader_bindings::time_num_pixels_y_label, utl::cast<f32>(render_size.y));
	}
}