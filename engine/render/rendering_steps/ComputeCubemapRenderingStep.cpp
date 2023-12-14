#include "ComputeCubemapRenderingStep.h"

#include <unordered_set>

#include <engine/render/ShaderObject.h>
#include <engine/profiler/Profiler.h>

#include <logger/logger.h>

using namespace engine::render;

CComputeCubemapRenderingStep::CComputeCubemapRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CComputeCubemapRenderingStep::_create(const FRenderingStepCreateInfo& create_info)
{
	TRACY_PROFILE_FUNCTION();

	auto& rendering_ci = create_info.rendering;

	uint32_t calculated_usage{};

	for (const auto& target : rendering_ci.renderTargets)
	{
		auto& info = m_pRenderContext->getTextureInfo(target);
		calculated_usage += info.mipLevels;
	}

	FComputeShaderCreateInfo shader_ci{};
	shader_ci.usageCount = (std::max)(calculated_usage, 1u);

	// Make RHI shader object
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface, m_pRenderContext);
	m_pShader->create(rendering_ci.shaderName, shader_ci);

	m_shaderInputs = rendering_ci.shaderInputs;

	// Bind default textures to shader
	for (auto& [binding_point, texture] : rendering_ci.shaderInputs)
		m_pShader->bindTexture(binding_point.c_str(), texture);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_beginRenderingInfo.name = std::format("compute_cubemap:{}", rendering_ci.shaderName);
#endif
}

void CComputeCubemapRenderingStep::_execute(float dt)
{
	TRACY_PROFILE_FUNCTION();

	m_pRenderContext->beginCompute(m_beginRenderingInfo);

	auto& render_packet = m_pRenderInterface->getRenderPacket();

	for (const auto& target : m_beginRenderingInfo.colorAttachments)
	{
		auto& info = m_pRenderContext->getTextureInfo(target);
		for (uint32_t mip = 0u; mip < info.mipLevels; ++mip)
		{
			// Bind textures
			for (auto& [name, texture] : m_shaderInputs)
			{
				uint32_t bind_mip{ 0u };

				if (texture == target)
					bind_mip = mip;

				m_pShader->bindTexture(name.c_str(), texture, bind_mip);
			}

			float layer_roughness = static_cast<float>(mip) / static_cast<float>(info.mipLevels - 1u);

			auto& uniform = m_pShader->getUniformBlock("ComputeCubemapData");
			if (uniform)
			{
				uniform->set("roughness", layer_roughness);
			}

			m_pShader->flushDescriptors();
			m_pShader->flushConstants(m_beginRenderingInfo.queue);
			m_pShader->bind(m_beginRenderingInfo.queue);

			auto& reflection = m_pShader->getReflection();

			FDispatchInfo dispatch_info{};
			auto& group = dispatch_info.groups.emplace_back();
			group.x = static_cast<uint32_t>(std::ceil(static_cast<float>(info.width) / static_cast<float>(reflection.localSizes[0])));
			group.y = static_cast<uint32_t>(std::ceil(static_cast<float>(info.height) / static_cast<float>(reflection.localSizes[1])));
			group.z = 0u;

			group.x = glm::max(group.x, 1u);
			group.y = glm::max(group.y, 1u);
			group.z = glm::max(group.z, 1u);

			m_pRenderContext->dispatch(m_beginRenderingInfo.queue, dispatch_info);
		}
	}

	m_pRenderContext->endCompute(m_beginRenderingInfo);
}