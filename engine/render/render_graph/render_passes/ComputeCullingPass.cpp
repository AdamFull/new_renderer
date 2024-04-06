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
	m_debugName = stl::format("compute_geometry:{}", m_baseShaderName);
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

	stl::unordered_set<u64> vbos{};

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
				shader->flushDescriptors();
				shader->bind(m_queue);
			}

			auto& references = shader->getConstantBlock("references");
			if (references)
			{
				//references->set("culling", render_world->getDrawBuffer(ERenderBufferIndices::eCullingInfoBuffer)->address());
				references->set("frustum", render_world->getDrawBuffer(ERenderBufferIndices::eFrustumBuffer)->buffer()->address());
			}

			shader->flushConstants(m_queue);

			auto& reflection = shader->getReflection();

			m_dispatchGroup.x = world_stats.drawCommandCount;

			FDispatch2Info dispatch_info{};
			dispatch_info.shaderHandle = shader->handle();
			dispatch_info.globalSizes = m_dispatchGroup;

			render_context->dispatch(m_queue, dispatch_info);
		}
	}
}

const stl::unique_ptr<IShaderObject>& CComputeCullingPass::getShaderByVBO(const IVertexHandler* vbo)
{
	auto& shader = m_shaders[vbo->getHash()];
	if (!shader)
	{
		shader = stl::make_unique<CShaderObject>(m_pRenderInterface);

		auto& render_packet = m_pRenderInterface->getRenderPacket();

		FComputeShaderCreateInfo shader_ci{};
		shader_ci.usageCount = stl::max(render_packet->stats.materialCount, render_packet->stats.primitiveCount);

		auto shader_name = m_baseShaderName + vbo->getDefinitions();
		shader->create(shader_name, shader_ci);

		initShaderResources(shader);
	}

	return shader;
}