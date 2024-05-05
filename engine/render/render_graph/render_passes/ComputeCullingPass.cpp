#include "ComputeCullingPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CComputeCullingPass::CComputeCullingPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputeCullingPass::init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::init(pass_ci);

	m_baseShaderName = pass_ci.shader;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("culling:{}", m_baseShaderName);
#endif
}

void CComputeCullingPass::build(const FPassBuildInfo& build_info)
{
	IComputePass::build(build_info);
}

void CComputeCullingPass::_execute(const FComputeFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& world_stats = render_world->getStats();

	std::unordered_set<u64> vbos{};

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
				if (auto& buffers = shader->getUniformBlock(shader_bindings::geometry_buffers); buffers)
				{
					u64 vertex_handle = last_vbo->getVertexHandle();
					auto vb_view = render_context->getBufferView(vertex_handle);
					buffers->set(shader_bindings::vb_label, vb_view.address);

					u64 index_buffer = last_vbo->getIndexHandle();
					auto ib_view = render_context->getBufferView(index_buffer);
					buffers->set(shader_bindings::ib_label, ib_view.address);

					buffers->set(shader_bindings::mb_label, last_vbo->getGeometryBuffer(EGeometryBufferIndices::eMeshletBuffer)->buffer()->address());
					buffers->set(shader_bindings::vrb_label, last_vbo->getGeometryBuffer(EGeometryBufferIndices::eVertexRemapBuffer)->buffer()->address());
					buffers->set(shader_bindings::bb_label, last_vbo->getGeometryBuffer(EGeometryBufferIndices::eBoundsBuffer)->buffer()->address());
					buffers->set(shader_bindings::tb_label, last_vbo->getGeometryBuffer(EGeometryBufferIndices::eTriangleBuffer)->buffer()->address());
					buffers->set(shader_bindings::lb_label, last_vbo->getGeometryBuffer(EGeometryBufferIndices::eLodBuffer)->buffer()->address());
					buffers->set(shader_bindings::last_index_label, utl::cast<u32>(last_vbo->getLastIndex()));
					buffers->set(shader_bindings::last_vertex_label, utl::cast<u32>(last_vbo->getLastVertex()));
				}

				shader->flushDescriptors();
				shader->bind(m_queue);
			}

			shader->flushConstants(m_queue);

			FDispatch2Info dispatch_info{};
			dispatch_info.divideByLocals = false;
			dispatch_info.globalSizes.x = world_stats.drawCommandCount;
			dispatch_info.globalSizes.y = 1;
			dispatch_info.globalSizes.z = 1;
			dispatch_info.shaderHandle = shader->handle();

			render_context->dispatch(m_queue, dispatch_info, 0);
		}
	}
}

const std::unique_ptr<IShaderObject>& CComputeCullingPass::getShaderByVBO(const IVertexHandler* vbo)
{
	auto& shader = m_shaders[vbo->getHash()];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface);

		auto& render_packet = m_pRenderInterface->getRenderPacket();

		FComputeShaderCreateInfo shader_ci{};
		shader_ci.usageCount = std::max(render_packet->stats.materialCount, render_packet->stats.primitiveCount);

		auto shader_name = m_baseShaderName + vbo->getDefinitions();
		shader->create(shader_name, shader_ci);

		initShaderResources(shader);
	}

	return shader;
}