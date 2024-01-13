#include "GeometryPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CGeometryPass::CGeometryPass(IRenderInterface* render_interface, IRenderContext* render_context, IRenderGraph* render_graph)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
	m_pRenderGraph = render_graph;
}

void CGeometryPass::init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::init(pass_ci);

	m_baseShaderName = pass_ci.shader;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("geometry:{}", m_baseShaderName);
#endif
}

void CGeometryPass::bake(std::unordered_map<size_t, FRenderGraphAttachmentOperationInfo>& resource_info)
{
	IRenderPass::bake(resource_info);
}

void CGeometryPass::_render(const FRenderFrameInfo& frame_info)
{
	m_pRenderContext->setScissorRect(0, 0, frame_info.renderRect.size.x, frame_info.renderRect.size.y);

	auto& render_world = m_pRenderInterface->getRenderWorld();

	auto& batch_ranges = render_world->getGeometryRanges(RENDER_GEOMETRY_CULLER_SCENE_TYPE);

	size_t drawcall_id{ 0ull };
	IVertexHandler* last_vbo{ nullptr };
	for (const auto& batch_range : batch_ranges)
	{
		for (size_t batch_id = batch_range.offset; batch_id < batch_range.offset + batch_range.count; ++batch_id)
		{
			const auto& batch = render_world->getGeometryBatch(batch_id);

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
					mesh_data->set("callID", static_cast<uint32_t>(drawcall_id));
					mesh_data->set("batchID", static_cast<uint32_t>(batch_id));
				}

				// Bind vbo and flush descriptors, then bind and draw

				shader->flushConstants(m_queue);

				m_pRenderContext->draw(m_queue, batch.command);
				drawcall_id += batch.command.count;
			}
		}
	}
}

const std::unique_ptr<IShaderObject>& CGeometryPass::getShaderByVBO(const IVertexHandler* vbo)
{
	auto vbo_hash = vbo->getHash();
	auto& shader = m_shaders[vbo_hash];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface, m_pRenderContext);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->getDefinitions();

		FGraphicsShaderCreateInfo shader_ci{};
		shader_ci.cullMode = m_bFlipViewport ? SHADER_CULL_MODE_BACK_BIT : SHADER_CULL_MODE_FRONT_BIT;
		shader_ci.frontFace = m_bFlipViewport ? SHADER_FRONT_FACE_CW : SHADER_FRONT_FACE_CCW;
		shader_ci.primitiveTopology = SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		
		auto& output_targets = m_renderResources[RENDER_RESOURCE_GROUP_COLOR_WRITE];
		for (size_t resource_id : output_targets)
		{
			auto& render_resource = m_pRenderGraph->getRenderResource(resource_id);
			shader_ci.colorAttachments.emplace_back(render_resource.handle);
		}

		if (m_depthTarget != ~0ull)
		{
			auto& render_resource = m_pRenderGraph->getRenderResource(m_depthTarget);
			shader_ci.depthAttachment = render_resource.handle;
		}

		if (m_stencilTarget != ~0ull)
		{
			auto& render_resource = m_pRenderGraph->getRenderResource(m_stencilTarget);
			shader_ci.stencilAttachment = render_resource.handle;
		}

		auto& render_packet = m_pRenderInterface->getRenderPacket();
		// TODO: i think i can calculate number of calls
		//shader_ci.usageCount = std::max(render_packet->stats.materialCount, render_packet->stats.primitiveCount);
		shader_ci.usageCount = render_packet->stats.perTypeVBOCount[vbo_hash];
		shader_ci.vertexDeclaration = vbo->getVertexDeclaration();

		shader->create(shader_name, shader_ci);

		// Bind default textures/buffers to shader
		for (const auto& [bind_point, resource_id] : m_bakedBindings)
		{
			auto& render_resource = m_pRenderGraph->getRenderResource(resource_id);
			if (render_resource.type == RENDER_RESOURCE_TYPE_RENDER_TARGET || render_resource.type == RENDER_RESOURCE_TYPE_STATIC_IMAGE)
				shader->bindTexture(bind_point.c_str(), render_resource.handle);
			else if (render_resource.type == RENDER_RESOURCE_TYPE_BUFFER)
				shader->bindBuffer(bind_point.c_str(), render_resource.handle);
		}
	}

	return shader;
}