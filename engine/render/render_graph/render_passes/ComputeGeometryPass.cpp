#include "ComputeGeometryPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CComputeGeometryPass::CComputeGeometryPass(IRenderInterface* render_interface, IRenderContext* render_context, IRenderGraph* render_graph)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
	m_pRenderGraph = render_graph;
}

void CComputeGeometryPass::init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::init(pass_ci);

	m_baseShaderName = pass_ci.shader;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("compute_geometry:{}", m_baseShaderName);
#endif
}

void CComputeGeometryPass::bake(std::unordered_map<size_t, FRenderGraphAttachmentOperationInfo>& resource_info)
{
	IComputePass::bake(resource_info);
}

void CComputeGeometryPass::_execute(const FComputeFrameInfo& frame_info)
{
	std::unordered_set<size_t> vbos{};

	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& batch_ranges = render_world->getGeometryRanges(RENDER_GEOMETRY_CULLER_SCENE_TYPE);
	for (const auto& batch_range : batch_ranges)
	{
		for (size_t batch_id = batch_range.offset; batch_id < batch_range.offset + batch_range.count; ++batch_id)
		{
			const auto& batch = render_world->getGeometryBatch(batch_id);
			vbos.emplace(batch.vboID);
		}
	}

	IVertexHandler* last_vbo{ nullptr };
	for (auto& vboId : vbos)
	{
		bool was_vbo_rebinded{ false };
		auto& vbo = m_pRenderInterface->getVertexHandler(vboId);
		if (!last_vbo || vbo.get() != last_vbo)
		{
			last_vbo = vbo.get();
			last_vbo->bind(m_queue);
			was_vbo_rebinded = true;
		}

		if (last_vbo)
		{
			auto& shader = getShaderByVBO(last_vbo);

			if (was_vbo_rebinded)
			{
				size_t vertex_handle = last_vbo->getVertexHandle();
				shader->bindBuffer("VertexBuffer", vertex_handle);

				size_t index_buffer = last_vbo->getIndexHandle();
				shader->bindBuffer("IndexBuffer", index_buffer);

				shader->flushDescriptors();
				shader->bind(m_queue);
			}

			auto& mesh_data = shader->getConstantBlock("meshData");
			if (mesh_data)
			{
				mesh_data->set("vboId", static_cast<uint32_t>(vboId));
			}

			shader->flushConstants(m_queue);

			auto& reflection = shader->getReflection();

			glm::vec3 global_size{};

			FDispatch2Info dispatch_info{};
			dispatch_info.globalSizes = m_dispatchGroup;
			dispatch_info.shaderHandle = shader->handle();

			m_pRenderContext->dispatch2(m_queue, dispatch_info);
		}
	}
}

const std::unique_ptr<IShaderObject>& CComputeGeometryPass::getShaderByVBO(const IVertexHandler* vbo)
{
	auto& shader = m_shaders[vbo->getHash()];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface, m_pRenderContext);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->getDefinitions();

		auto& render_packet = m_pRenderInterface->getRenderPacket();

		FComputeShaderCreateInfo shader_ci{};
		shader_ci.usageCount = std::max(render_packet->stats.materialCount, render_packet->stats.primitiveCount);
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