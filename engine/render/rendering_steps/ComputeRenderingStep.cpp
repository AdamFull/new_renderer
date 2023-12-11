#include "ComputeRenderingStep.h"

#include <unordered_set>

#include <engine/render/ShaderObject.h>
#include <engine/profiler/Profiler.h>

#include <logger/logger.h>

using namespace engine::render;

CComputeRenderingStep::CComputeRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CComputeRenderingStep::create(const FRenderingStepCreateInfo& create_info)
{
	TRACY_PROFILE_FUNCTION();

	auto& rendering_ci = create_info.rendering;

	// Save rendering info
	m_beginRenderingInfo.queue = rendering_ci.queue;
	m_beginRenderingInfo.colorAttachments = rendering_ci.renderTargets;
	m_beginRenderingInfo.depthAttachment = rendering_ci.depthTarget;
	m_beginRenderingInfo.stencilAttachment = rendering_ci.stencilTarget;
	m_beginRenderingInfo.flipViewport = rendering_ci.flipViewport;

	m_shaderInputs = rendering_ci.shaderInputs;

	m_baseShaderName = rendering_ci.shaderName;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_beginRenderingInfo.name = std::format("compute:{}", rendering_ci.shaderName);
#endif
}

void CComputeRenderingStep::execute(float dt)
{
	TRACY_PROFILE_FUNCTION();

	auto render_resolution = m_pRenderContext->getRenderResolution();

	auto& render_packet = m_pRenderInterface->getRenderPacket();

	std::vector<FGeometryBatch> rebatched{};
	rebatchGeometry(render_packet->sceneBatches, rebatched, GEOMETRY_BATCHER_MERGE_DRAWCALLS_FLAG_BIT);

	auto& materials = render_packet->materials;
	auto& nodes = render_packet->nodes;
	auto& skins = render_packet->skins;

	auto& dlights = render_packet->dlights.lights;
	auto& plights = render_packet->plights.lights;
	auto& slights = render_packet->slights.lights;

	IVertexHandler* last_vbo{ nullptr };
	for (auto& batch : rebatched)
	{
		auto& vbo = m_pRenderInterface->getVertexHandler(batch.vboId);
		if (!last_vbo || vbo.get() != last_vbo)
		{
			last_vbo = vbo.get();
			last_vbo->bind(m_beginRenderingInfo.queue);
		}

		if (last_vbo)
		{
			auto& shader = getShaderByVBO(last_vbo);

			auto& ubo = shader->getUniformBlock("UniformMeshData");
			if (ubo)
			{
				ubo->set("vboId", static_cast<uint32_t>(batch.vboId));
				ubo->set("objectId", batch.node);
				ubo->set("materialId", static_cast<uint32_t>(batch.materialId));
			}

			if (materials)
			{
				auto& buffer = materials->getBufferObject();
				size_t handle = buffer->handle();
				shader->bindBuffer("MaterialStorage", handle);
			}

			if (nodes)
			{
				auto& buffer = nodes->getBufferObject();
				size_t handle = buffer->handle();
				shader->bindBuffer("NodeStorage", handle);
			}

			if (skins)
			{
				auto& buffer = skins->getBufferObject();
				size_t handle = buffer->handle();
				shader->bindBuffer("SkinningJoints", handle);
			}


			if (dlights)
			{
				auto& buffer = dlights->getBufferObject();
				size_t handle = buffer->handle();
				shader->bindBuffer("DirectionalLights", handle);
			}

			if (plights)
			{
				auto& buffer = plights->getBufferObject();
				size_t handle = buffer->handle();
				shader->bindBuffer("PointLights", handle);
			}

			if (slights)
			{
				auto& buffer = slights->getBufferObject();
				size_t handle = buffer->handle();
				shader->bindBuffer("SpotLights", handle);
			}

			size_t vertex_handle = last_vbo->getVertexHandle();
			shader->bindBuffer("VertexBuffer", vertex_handle);

			size_t index_buffer = last_vbo->getIndexHandle();
			shader->bindBuffer("IndexBuffer", index_buffer);

			shader->flushDescriptors();
			shader->flushConstants(m_beginRenderingInfo.queue);
			shader->bind(m_beginRenderingInfo.queue);

			auto& reflection = shader->getReflection();
			
			FDispatchInfo dispatch_info{};
			auto& group = dispatch_info.groups.emplace_back();
			group.x = static_cast<uint32_t>(std::ceil(static_cast<float>(render_resolution.x) / static_cast<float>(reflection.localSizes[0])));
			group.y = static_cast<uint32_t>(std::ceil(static_cast<float>(render_resolution.y) / static_cast<float>(reflection.localSizes[1])));
			group.z = 0u;
			
			group.x = glm::max(group.x, 1u);
			group.y = glm::max(group.y, 1u);
			group.z = glm::max(group.z, 1u);
			
			m_pRenderContext->dispatch(m_beginRenderingInfo.queue, dispatch_info);
		}
	}
}

const std::unique_ptr<IShaderObject>& CComputeRenderingStep::getShaderByVBO(const IVertexHandler* vbo)
{
	auto& shader = m_shaders[vbo->getHash()];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface, m_pRenderContext);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->getDefinitions();

		auto& render_packet = m_pRenderInterface->getRenderPacket();

		FComputeShaderCreateInfo shader_ci{};
		shader_ci.usageCount = std::max(render_packet->materials->getLastInsertionIndex() + 1ull, render_packet->primitiveCount);
		shader->create(shader_name, shader_ci);

		for (auto& [binding_point, texture] : m_shaderInputs)
			shader->bindTexture(binding_point.c_str(), texture);
	}

	return shader;
}