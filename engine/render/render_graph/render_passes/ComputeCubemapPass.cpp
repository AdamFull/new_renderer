#include "ComputeCubemapPass.h"

#include <engine/render/ShaderObject.h>

using namespace engine::render;

CComputeCubemapPass::CComputeCubemapPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputeCubemapPass::Init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::Init(pass_ci);

	u32 calculated_usage{};

	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	auto& out_targets = m_renderResources[ERenderResourceGroupType::eStorageTextureWrite];
	for (const auto& resource_binding : out_targets)
	{
		auto& render_resource = render_graph->GetRenderResource(resource_binding.resource);
		auto& info = render_context->GetTextureInfo(render_resource.handle);
		calculated_usage += info.mipLevels;
	}

	FComputeShaderCreateInfo shader_ci{};
	shader_ci.usageCount = (std::max)(calculated_usage, 1u);

	// Make RHI shader object
	m_pShader = std::make_unique<CShaderObject>(m_pRenderInterface);
	m_pShader->Create(pass_ci.shader, shader_ci);

	InitShaderResources(m_pShader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("compute_cubemap:{}", pass_ci.shader);
#endif
}

void CComputeCubemapPass::Build(const FPassBuildInfo& build_info)
{
	IComputePass::Build(build_info);
}

void CComputeCubemapPass::_Execute(const FComputeFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& render_graph = m_pRenderInterface->GetRenderGraph();
	auto& render_packet = m_pRenderInterface->GetRenderPacket();

	auto& write_resources = m_renderResources[ERenderResourceGroupType::eStorageTextureWrite];

	for (const auto& resource_binding : write_resources)
	{
		auto& render_resource = render_graph->GetRenderResource(resource_binding.resource);
		auto& info = render_context->GetTextureInfo(render_resource.handle);
		for (u32 mip = 0u; mip < info.mipLevels; ++mip)
		{
			for (const auto& [binding_point, sesource_binding] : m_bakedBindings)
			{
				auto& binding_resource = render_graph->GetRenderResource(sesource_binding.resource);

				if (binding_resource.handle == render_resource.handle)
					m_pShader->BindTexture(utl::fnv1a_64_hash_cstr(binding_point.c_str()), binding_resource.handle, mip == 0u ? ~0u : mip);
				else
					m_pShader->BindTexture(utl::fnv1a_64_hash_cstr(binding_point.c_str()), binding_resource.handle);
			}

			f32 layer_roughness = utl::cast<f32>(mip) / utl::cast<f32>(info.mipLevels - 1u);

			auto& uniform = m_pShader->GetUniformBlock(shader_bindings::compute_cubemap_data);
			if (uniform)
			{
				uniform->Set(shader_bindings::roughness_label, layer_roughness);
			}

			m_pShader->FlushDescriptors();
			m_pShader->FlushConstants(m_queue);
			m_pShader->Bind(m_queue);

			auto& dispatch_info = m_vDispatchGroups[0];
			dispatch_info.globalSizes.x = info.size.width;
			dispatch_info.globalSizes.y = info.size.height;
			dispatch_info.shaderHandle = m_pShader->Handle();
			
			render_context->Dispatch(m_queue, dispatch_info, 0);
		}
	}
}