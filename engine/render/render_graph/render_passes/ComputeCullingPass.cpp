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
	m_debugName = std::format("compute_geometry:{}", m_baseShaderName);
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

	for (const auto& [type, batch_ranges] : render_world->getAllGeometryRanges())
	{
		for (const auto& batch_range : batch_ranges)
		{
			for (u64 batch_id = batch_range.offset; batch_id < batch_range.offset + batch_range.count; ++batch_id)
			{
				const auto& batch = render_world->getGeometryBatch(batch_id);
				vbos.emplace(batch.vboID);
			}
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
				shader->flushDescriptors();
				shader->bind(m_queue);
			}

			auto& references = shader->getConstantBlock("references");
			if (references)
			{
				auto& buffer_mappings = render_world->getDrawBuffers();
				references->set("culling", buffer_mappings[culling_info_buffer_index]->address());
				references->set("frustum", buffer_mappings[frustum_buffer_index]->address());
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
	}

	return shader;
}