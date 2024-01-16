#include "ComputeCubemapPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CComputeCubemapPass::CComputeCubemapPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputeCubemapPass::init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::init(pass_ci);

	uint32_t calculated_usage{};

	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto& out_targets = m_renderResources[ERenderResourceGroupType::eStorageWrite];
	for (size_t resource_id : out_targets)
	{
		auto& render_resource = render_graph->getRenderResource(resource_id);
		auto& info = render_context->getTextureInfo(render_resource.handle);
		calculated_usage += info.mipLevels;
	}

	FComputeShaderCreateInfo shader_ci{};
	shader_ci.usageCount = (std::max)(calculated_usage, 1u);

	// Make RHI shader object
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface);
	m_pShader->create(pass_ci.shader, shader_ci);

	// Bind default textures/buffers to shader
	for (const auto& [bind_point, resource_id] : m_bakedBindings)
	{
		auto& render_resource = render_graph->getRenderResource(resource_id);
		if (render_resource.type & ERenderResourceTypeFlagBits::eAllImage)
			m_pShader->bindTexture(bind_point.c_str(), render_resource.handle);
		else if (render_resource.type & ERenderResourceTypeFlagBits::eBuffer)
			m_pShader->bindBuffer(bind_point.c_str(), render_resource.handle);
	}

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("compute_cubemap:{}", pass_ci.shader);
#endif
}

void CComputeCubemapPass::bake(std::unordered_map<size_t, FRenderGraphAttachmentOperationInfo>& resource_info)
{
	IComputePass::bake(resource_info);
}

void CComputeCubemapPass::_execute(const FComputeFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_graph = m_pRenderInterface->getRenderGraph();
	auto& render_packet = m_pRenderInterface->getRenderPacket();

	auto& write_resources = m_renderResources[ERenderResourceGroupType::eStorageWrite];
	for (const auto& resource_id : write_resources)
	{
		auto& render_resource = render_graph->getRenderResource(resource_id);
		auto& info = render_context->getTextureInfo(render_resource.handle);
		for (uint32_t mip = 0u; mip < info.mipLevels; ++mip)
		{
			for (const auto& [binding_point, resource_id] : m_bakedBindings)
			{
				auto& binding_resource = render_graph->getRenderResource(resource_id);

				if(binding_resource.handle == render_resource.handle)
					m_pShader->bindTexture(binding_point.c_str(), binding_resource.handle, mip);
				else
					m_pShader->bindTexture(binding_point.c_str(), binding_resource.handle);
			}

			float layer_roughness = static_cast<float>(mip) / static_cast<float>(info.mipLevels - 1u);

			auto& uniform = m_pShader->getUniformBlock("ComputeCubemapData");
			if (uniform)
			{
				uniform->set("roughness", layer_roughness);
			}

			m_pShader->flushDescriptors();
			m_pShader->flushConstants(m_queue);
			m_pShader->bind(m_queue);

			auto& reflection = m_pShader->getReflection();

			FDispatch2Info dispatch_info{};
			dispatch_info.shaderHandle = m_pShader->handle();
			dispatch_info.globalSizes.x = info.width;
			dispatch_info.globalSizes.y = info.height;

			render_context->dispatch2(m_queue, dispatch_info);
		}
	}
}