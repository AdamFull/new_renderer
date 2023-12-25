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

	auto& render_packet = m_pRenderInterface->getRenderPacket();

	auto& materials = render_packet->materials;
	auto& nodes = render_packet->nodes;
	auto& skins = render_packet->skins;

	auto& dlights = render_packet->dlights.lights;
	auto& plights = render_packet->plights.lights;
	auto& slights = render_packet->slights.lights;

	std::unordered_set<size_t> vbos{};
	for (const auto& batch : render_packet->sceneDrawCalls)
		vbos.emplace(batch.vboId);

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

				auto& dshadows = render_packet->dlights.shadows;
				if (dshadows)
				{
					auto& buffer = dshadows->getBufferObject();
					size_t handle = buffer->handle();
					shader->bindBuffer("CascadedShadows", handle);
				}

				auto& pshadows = render_packet->plights.shadows;
				if (pshadows)
				{
					auto& buffer = pshadows->getBufferObject();
					size_t handle = buffer->handle();
					shader->bindBuffer("OmniShadows", handle);
				}

				auto& sshadows = render_packet->slights.shadows;
				if (sshadows)
				{
					auto& buffer = sshadows->getBufferObject();
					size_t handle = buffer->handle();
					shader->bindBuffer("SpotShadows", handle);
				}

				size_t vertex_handle = last_vbo->getVertexHandle();
				shader->bindBuffer("VertexBuffer", vertex_handle);

				size_t index_buffer = last_vbo->getIndexHandle();
				shader->bindBuffer("IndexBuffer", index_buffer);

				size_t indirect_buffer = render_packet->drawIndexedCommands->handle();
				shader->bindBuffer("IndirectIndexedComands", indirect_buffer);

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