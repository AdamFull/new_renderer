#include "ComputeCubemapPass.h"

#include <engine/render/ShaderObject.h>

using namespace engine::render;

CComputeCubemapPass::CComputeCubemapPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputeCubemapPass::init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::init(pass_ci);

	u32 calculated_usage{};

	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_graph = m_pRenderInterface->getRenderGraph();

	auto& out_targets = m_renderResources[ERenderResourceGroupType::eStorageTextureWrite];
	for (const auto& resource_binding : out_targets)
	{
		auto& render_resource = render_graph->getRenderResource(resource_binding.resource);
		auto& info = render_context->getTextureInfo(render_resource.handle);
		calculated_usage += info.mipLevels;
	}

	FComputeShaderCreateInfo shader_ci{};
	shader_ci.usageCount = (std::max)(calculated_usage, 1u);

	// Make RHI shader object
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface);
	m_pShader->create(pass_ci.shader, shader_ci);

	initShaderResources(m_pShader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("compute_cubemap:{}", pass_ci.shader);
#endif
}

void CComputeCubemapPass::build(const FPassBuildInfo& build_info)
{
	IComputePass::build(build_info);
}

void CComputeCubemapPass::_execute(const FComputeFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_graph = m_pRenderInterface->getRenderGraph();
	auto& render_packet = m_pRenderInterface->getRenderPacket();

	auto& write_resources = m_renderResources[ERenderResourceGroupType::eStorageTextureWrite];

	for (const auto& resource_binding : write_resources)
	{
		auto& render_resource = render_graph->getRenderResource(resource_binding.resource);
		auto& info = render_context->getTextureInfo(render_resource.handle);
		for (u32 mip = 0u; mip < info.mipLevels; ++mip)
		{
			for (const auto& [binding_point, sesource_binding] : m_bakedBindings)
			{
				auto& binding_resource = render_graph->getRenderResource(sesource_binding.resource);

				if (binding_resource.handle == render_resource.handle)
					m_pShader->bindTexture(utl::utils::fnv1a_64_hash(binding_point.c_str()), binding_resource.handle, mip == 0u ? ~0u : mip);
				else
					m_pShader->bindTexture(utl::utils::fnv1a_64_hash(binding_point.c_str()), binding_resource.handle);
			}

			f32 layer_roughness = utl::cast<f32>(mip) / utl::cast<f32>(info.mipLevels - 1u);

			auto& uniform = m_pShader->getUniformBlock(shader_bindings::compute_cubemap_data);
			if (uniform)
			{
				uniform->set(shader_bindings::roughness_label, layer_roughness);
			}

			m_pShader->flushDescriptors();
			m_pShader->flushConstants(m_queue);
			m_pShader->bind(m_queue);

			auto& dispatch_info = m_vDispatchGroups[0];
			dispatch_info.globalSizes.x = info.size.width;
			dispatch_info.globalSizes.y = info.size.height;
			dispatch_info.shaderHandle = m_pShader->handle();
			
			render_context->dispatch(m_queue, dispatch_info, 0);
		}
	}
}