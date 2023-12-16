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

void CComputeRenderingStep::_create(const FRenderingStepCreateInfo& create_info)
{
	TRACY_PROFILE_FUNCTION();

	auto& rendering_ci = create_info.rendering;

	FComputeShaderCreateInfo shader_ci{};

	// Make RHI shader object
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface, m_pRenderContext);
	m_pShader->create(rendering_ci.shaderName, shader_ci);

	// Bind default textures to shader
	for (auto& [binding_point, texture] : rendering_ci.shaderInputs)
		m_pShader->bindTexture(binding_point.c_str(), texture);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_beginRenderingInfo.name = std::format("compute:{}", rendering_ci.shaderName);
#endif
}

void CComputeRenderingStep::_execute(float dt)
{
	TRACY_PROFILE_FUNCTION();

	m_pRenderContext->beginCompute(m_beginRenderingInfo);

	auto& rect = m_beginRenderingInfo.renderArea.size;

	auto& render_packet = m_pRenderInterface->getRenderPacket();

	auto& dlights = render_packet->dlights.lights;
	if (dlights)
	{
		auto& buffer = dlights->getBufferObject();
		size_t handle = buffer->handle();
		m_pShader->bindBuffer("DirectionalLights", handle);
	}

	auto& plights = render_packet->plights.lights;
	if (plights)
	{
		auto& buffer = plights->getBufferObject();
		size_t handle = buffer->handle();
		m_pShader->bindBuffer("PointLights", handle);
	}

	auto& slights = render_packet->slights.lights;
	if (slights)
	{
		auto& buffer = slights->getBufferObject();
		size_t handle = buffer->handle();
		m_pShader->bindBuffer("SpotLights", handle);
	}

	auto& dshadows = render_packet->dlights.shadows;
	if (dshadows)
	{
		auto& buffer = dshadows->getBufferObject();
		size_t handle = buffer->handle();
		m_pShader->bindBuffer("CascadedShadows", handle);
	}

	auto& pshadows = render_packet->plights.shadows;
	if (pshadows)
	{
		auto& buffer = pshadows->getBufferObject();
		size_t handle = buffer->handle();
		m_pShader->bindBuffer("OmniShadows", handle);
	}

	auto& sshadows = render_packet->slights.shadows;
	if (sshadows)
	{
		auto& buffer = sshadows->getBufferObject();
		size_t handle = buffer->handle();
		m_pShader->bindBuffer("SpotShadows", handle);
	}

	m_pShader->flushDescriptors();
	m_pShader->flushConstants(m_beginRenderingInfo.queue);
	m_pShader->bind(m_beginRenderingInfo.queue);

	auto& reflection = m_pShader->getReflection();

	FDispatchInfo dispatch_info{};
	auto& group = dispatch_info.groups.emplace_back();
	group.x = static_cast<uint32_t>(std::ceil(static_cast<float>(rect.x) / static_cast<float>(reflection.localSizes[0])));
	group.y = static_cast<uint32_t>(std::ceil(static_cast<float>(rect.y) / static_cast<float>(reflection.localSizes[1])));
	group.z = 0u;

	group.x = glm::max(group.x, 1u);
	group.y = glm::max(group.y, 1u);
	group.z = glm::max(group.z, 1u);

	m_pRenderContext->dispatch(m_beginRenderingInfo.queue, dispatch_info);

	m_pRenderContext->endCompute(m_beginRenderingInfo);
}