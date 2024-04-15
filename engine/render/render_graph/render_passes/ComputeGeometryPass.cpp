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
	m_debugName = std::format("compute_geometry:{}", m_baseShaderName);
#endif
}

void CComputeGeometryPass::build(const FPassBuildInfo& build_info)
{
	IComputePass::build(build_info);
}

void CComputeGeometryPass::_execute(const FComputeFrameInfo& frame_info)
{
	std::unordered_set<u64> vbos{};

	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_world = m_pRenderInterface->getRenderWorld();


	const auto& geometry_batches = render_world->getGeometryBatches();
	for (const auto& batch : geometry_batches)
		vbos.emplace(batch.vboID);

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
				bindShaderResources(shader, frame_info.deltaTime);
				if (auto& buffers = shader->getUniformBlock("GeometryBuffers"); buffers)
				{
					u64 vertex_handle = last_vbo->getVertexHandle();
					auto vb_view = render_context->getBufferView(vertex_handle);
					buffers->set("vb", vb_view.address);

					u64 index_buffer = last_vbo->getIndexHandle();
					auto ib_view = render_context->getBufferView(index_buffer);
					buffers->set("ib", ib_view.address);

					buffers->set("mb", last_vbo->getGeometryBuffer(EGeometryBufferIndices::eMeshletBuffer)->buffer()->address());
					buffers->set("vrb", last_vbo->getGeometryBuffer(EGeometryBufferIndices::eVertexRemapBuffer)->buffer()->address());
					buffers->set("bb", last_vbo->getGeometryBuffer(EGeometryBufferIndices::eBoundsBuffer)->buffer()->address());
					buffers->set("tb", last_vbo->getGeometryBuffer(EGeometryBufferIndices::eTriangleBuffer)->buffer()->address());
					buffers->set("lb", last_vbo->getGeometryBuffer(EGeometryBufferIndices::eLodBuffer)->buffer()->address());
				}

				shader->flushDescriptors();
				shader->bind(m_queue);
			}

			auto& mesh_data = shader->getConstantBlock("meshData");
			if (mesh_data)
			{
				mesh_data->set("vboId", utl::cast<u32>(vboId));
			}

			shader->flushConstants(m_queue);

			auto& dispatch_info = m_vDispatchGroups.front();
			dispatch_info.shaderHandle = shader->handle();

			render_context->dispatch(m_queue, dispatch_info, 0);
		}
	}
}

const std::unique_ptr<IShaderObject>& CComputeGeometryPass::getShaderByVBO(const IVertexHandler* vbo)
{
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto& shader = m_shaders[vbo->getHash()];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->getDefinitions();

		auto& render_packet = m_pRenderInterface->getRenderPacket();

		FComputeShaderCreateInfo shader_ci{};
		shader_ci.usageCount = std::max(render_packet->stats.materialCount, render_packet->stats.primitiveCount);
		shader->create(shader_name, shader_ci);

		initShaderResources(shader);
	}

	return shader;
}