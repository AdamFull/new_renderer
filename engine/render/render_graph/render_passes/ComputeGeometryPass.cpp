#include "ComputeGeometryPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CComputeGeometryPass::CComputeGeometryPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputeGeometryPass::init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::init(pass_ci);

	m_baseShaderName = pass_ci.shader;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = stl::format("compute_geometry:{}", m_baseShaderName);
#endif
}

void CComputeGeometryPass::build(const FPassBuildInfo& build_info)
{
	IComputePass::build(build_info);
}

void CComputeGeometryPass::_execute(const FComputeFrameInfo& frame_info)
{
	stl::unordered_set<u64> vbos{};

	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_world = m_pRenderInterface->getRenderWorld();

	auto& batch_ranges = render_world->getGeometryRanges(ERenderGeometryCullerType::eScene);
	for (const auto& batch_range : batch_ranges)
	{
		for (u64 batch_id = batch_range.offset; batch_id < batch_range.offset + batch_range.count; ++batch_id)
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
				u64 vertex_handle = last_vbo->getVertexHandle();
				shader->bindBuffer("VertexBuffer", vertex_handle);

				u64 index_buffer = last_vbo->getIndexHandle();
				shader->bindBuffer("IndexBuffer", index_buffer);

				shader->flushDescriptors();
				shader->bind(m_queue);
			}

			auto& mesh_data = shader->getConstantBlock("meshData");
			if (mesh_data)
			{
				mesh_data->set("vboId", utl::cast<u32>(vboId));
			}

			shader->flushConstants(m_queue);

			auto& reflection = shader->getReflection();

			glm::vec3 global_size{};

			FDispatch2Info dispatch_info{};
			dispatch_info.globalSizes = m_dispatchGroup;
			dispatch_info.shaderHandle = shader->handle();

			render_context->dispatch(m_queue, dispatch_info);
		}
	}
}

const stl::unique_ptr<IShaderObject>& CComputeGeometryPass::getShaderByVBO(const IVertexHandler* vbo)
{
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto& shader = m_shaders[vbo->getHash()];
	if (!shader)
	{
		shader = stl::make_unique<CShaderObject>(m_pRenderInterface);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->getDefinitions();

		auto& render_packet = m_pRenderInterface->getRenderPacket();

		FComputeShaderCreateInfo shader_ci{};
		shader_ci.usageCount = stl::max(render_packet->stats.materialCount, render_packet->stats.primitiveCount);
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