#include "ComputeGeometryRenderingStep.h"

#include <unordered_set>

#include <engine/render/ShaderObject.h>
#include <engine/profiler/Profiler.h>

#include <logger/logger.h>

using namespace engine::render;

CComputeGeometryRenderingStep::CComputeGeometryRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CComputeGeometryRenderingStep::_create(const FRenderingStepCreateInfo& create_info)
{
	TRACY_PROFILE_FUNCTION();

	auto& rendering_ci = create_info.compute;

	m_shaderInputs.insert(rendering_ci.shaderInputs.begin(), rendering_ci.shaderInputs.end());
	m_shaderInputs.insert(rendering_ci.writeTargets.begin(), rendering_ci.writeTargets.end());

	m_baseShaderName = rendering_ci.shaderName;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_beginComputeInfo.name = std::format("compute_geometry:{}", rendering_ci.shaderName);
#endif
}

void CComputeGeometryRenderingStep::_execute(float dt)
{
	TRACY_PROFILE_FUNCTION();

	m_pRenderContext->beginCompute(m_beginComputeInfo);

	auto& sizes = m_beginComputeInfo.globalSizes;

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
			last_vbo->bind(m_beginComputeInfo.queue);
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
				shader->bind(m_beginComputeInfo.queue);
			}

			auto& mesh_data = shader->getConstantBlock("meshData");
			if (mesh_data)
			{
				mesh_data->set("vboId", static_cast<uint32_t>(vboId));
				//mesh_data->set("objectId", batch.node);
				//mesh_data->set("materialId", static_cast<uint32_t>(batch.materialId));
			}

			shader->flushConstants(m_beginComputeInfo.queue);

			auto& reflection = shader->getReflection();
			
			FDispatchInfo dispatch_info{};
			auto& group = dispatch_info.groups.emplace_back();
			group.x = glm::max(static_cast<uint32_t>(std::ceil(static_cast<float>(sizes.x) / static_cast<float>(reflection.localSizes[0]))), 1u);
			group.y = glm::max(static_cast<uint32_t>(std::ceil(static_cast<float>(sizes.y) / static_cast<float>(reflection.localSizes[1]))), 1u);
			group.z = glm::max(static_cast<uint32_t>(std::ceil(static_cast<float>(sizes.z) / static_cast<float>(reflection.localSizes[2]))), 1u);
			
			m_pRenderContext->dispatch(m_beginComputeInfo.queue, dispatch_info);
			//break;
		}
	}

	m_pRenderContext->endCompute(m_beginComputeInfo);
}

const std::unique_ptr<IShaderObject>& CComputeGeometryRenderingStep::getShaderByVBO(const IVertexHandler* vbo)
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

		for (auto& [binding_point, texture] : m_shaderInputs)
			shader->bindTexture(binding_point.c_str(), texture);
	}

	return shader;
}