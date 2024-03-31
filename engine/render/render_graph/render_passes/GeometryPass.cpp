#include "GeometryPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CGeometryPass::CGeometryPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CGeometryPass::init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::init(pass_ci);

	m_baseShaderName = pass_ci.shader;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = stl::format("geometry:{}", m_baseShaderName);
#endif
}

void CGeometryPass::build(const FPassBuildInfo& build_info)
{
	IRenderPass::build(build_info);
}

void CGeometryPass::_render(const FRenderFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->setScissorRect(0, 0, frame_info.renderRect.size.x, frame_info.renderRect.size.y);

	auto& render_world = m_pRenderInterface->getRenderWorld();

	const auto& geometry_batches = render_world->getGeometryBatches();

	u64 drawcall_id{ 0ull };
	IVertexHandler* last_vbo{ nullptr };
	for (const auto& batch : geometry_batches)
	{
		bool was_vbo_rebinded{ false };
		auto& vbo = m_pRenderInterface->getVertexHandler(batch.vboID);
		if (!last_vbo || vbo.get() != last_vbo)
		{
			last_vbo = vbo.get();
			last_vbo->bind(m_queue);
			was_vbo_rebinded = true;
		}

		if (last_vbo)
		{
			// TODO: this code looks very ugly as i think
			// Create shader for vertex declaration if not exists
			auto& shader = getShaderByVBO(last_vbo);

			if (was_vbo_rebinded)
			{
				shader->flushDescriptors();
				shader->bind(m_queue);
			}

			auto& mesh_data = shader->getConstantBlock("meshData");
			if (mesh_data)
			{
				mesh_data->set("callID", utl::cast<u32>(drawcall_id));
				mesh_data->set("batchID", utl::cast<u32>(0));
			}

			// Bind vbo and flush descriptors, then bind and draw

			shader->flushConstants(m_queue);

			render_context->draw(m_queue, batch.command);
			drawcall_id += batch.command.count;
		}
	}
}

const stl::unique_ptr<IShaderObject>& CGeometryPass::getShaderByVBO(const IVertexHandler* vbo)
{
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto vbo_hash = vbo->getHash();
	auto& shader = m_shaders[vbo_hash];
	if (!shader)
	{
		shader = stl::make_unique<CShaderObject>(m_pRenderInterface);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->getDefinitions();

		FGraphicsShaderCreateInfo shader_ci{};
		shader_ci.cullMode = m_bFlipViewport ? EShaderCullModeFlagBits::eBack : EShaderCullModeFlagBits::eFront;
		shader_ci.frontFace = m_bFlipViewport ? EShaderFrontFace::eCW : EShaderFrontFace::eCCW;
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

		auto& render_packet = m_pRenderInterface->getRenderPacket();
		// TODO: i think i can calculate number of calls
		//shader_ci.usageCount = stl::max(render_packet->stats.materialCount, render_packet->stats.primitiveCount);
		shader_ci.usageCount = render_packet->stats.perTypeVBOCount[vbo_hash];
		shader_ci.vertexDeclaration = vbo->getVertexDeclaration();

		shader->create(shader_name, shader_ci);

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

	return shader;
}