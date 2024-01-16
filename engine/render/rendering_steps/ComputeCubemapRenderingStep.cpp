#include "ComputeCubemapRenderingStep.h"

#include <unordered_set>

#include <engine/render/ShaderObject.h>
#include <engine/profiler/Profiler.h>

#include <logger/logger.h>

using namespace engine::render;

CComputeCubemapRenderingStep::CComputeCubemapRenderingStep(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputeCubemapRenderingStep::_create(const FRenderingStepCreateInfo& create_info)
{
	TRACY_PROFILE_FUNCTION();

	auto& rendering_ci = create_info.compute;
	auto& render_context = m_pRenderInterface->getRenderContext();

	u32 calculated_usage{};

	for (const auto& [name, target] : rendering_ci.writeTargets)
	{
		auto& info = render_context->getTextureInfo(target);
		calculated_usage += info.mipLevels;
	}

	FComputeShaderCreateInfo shader_ci{};
	shader_ci.usageCount = (std::max)(calculated_usage, 1u);

	// Make RHI shader object
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface);
	m_pShader->create(rendering_ci.shaderName, shader_ci);

	m_shaderInputs.insert(rendering_ci.shaderInputs.begin(), rendering_ci.shaderInputs.end());
	m_shaderInputs.insert(rendering_ci.writeTargets.begin(), rendering_ci.writeTargets.end());

	// Bind default textures to shader
	for (auto& [binding_point, texture] : rendering_ci.shaderInputs)
		m_pShader->bindTexture(binding_point.c_str(), texture);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_beginComputeInfo.name = std::format("compute_cubemap:{}", rendering_ci.shaderName);
#endif
}

void CComputeCubemapRenderingStep::_execute(f32 dt)
{
	TRACY_PROFILE_FUNCTION();

	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->beginCompute(m_beginComputeInfo);

	auto& render_packet = m_pRenderInterface->getRenderPacket();

	for (const auto& target : m_beginComputeInfo.writeAttachments)
	{
		auto& info = render_context->getTextureInfo(target);
		for (u32 mip = 0u; mip < info.mipLevels; ++mip)
		{
			// Bind textures
			for (auto& [name, texture] : m_shaderInputs)
			{
				u32 bind_mip{ 0u };

				if (texture == target)
					bind_mip = mip;

				m_pShader->bindTexture(name.c_str(), texture, bind_mip);
			}

			f32 layer_roughness = utl::cast<f32>(mip) / utl::cast<f32>(info.mipLevels - 1u);

			auto& uniform = m_pShader->getUniformBlock("ComputeCubemapData");
			if (uniform)
			{
				uniform->set("roughness", layer_roughness);
			}

			m_pShader->flushDescriptors();
			m_pShader->flushConstants(m_beginComputeInfo.queue);
			m_pShader->bind(m_beginComputeInfo.queue);

			auto& reflection = m_pShader->getReflection();

			FDispatchInfo dispatch_info{};
			auto& group = dispatch_info.groups.emplace_back();
			group.x = glm::max(utl::cast<u32>(std::ceil(utl::cast<f32>(info.width) / utl::cast<f32>(reflection.localSizes[0]))), 1u);
			group.y = glm::max(utl::cast<u32>(std::ceil(utl::cast<f32>(info.height) / utl::cast<f32>(reflection.localSizes[1]))), 1u);
			group.z = glm::max(0u, 1u);

			render_context->dispatch(m_beginComputeInfo.queue, dispatch_info);
		}
	}

	render_context->endCompute(m_beginComputeInfo);
}