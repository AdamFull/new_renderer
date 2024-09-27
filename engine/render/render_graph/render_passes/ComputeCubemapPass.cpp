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

	auto& render_backend = m_pRenderInterface->GetRenderBackend();
	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	auto& out_targets = m_renderResources[ERenderResourceGroupType::eStorageTextureWrite];
	for (const auto& resource_binding : out_targets)
	{
		auto& render_resource = render_graph->GetRenderResource(resource_binding.resource);
		auto texture_view = render_backend->GetTextureView(render_resource.handle);
		calculated_usage += texture_view.mipLevels;
	}

	FComputeShaderCreateInfo shader_ci{};
	shader_ci.usageCount = (std::max)(calculated_usage, 1u);
	m_shader = m_pRenderInterface->CreateShaderObject(pass_ci.shader, shader_ci);

	InitShaderResources(m_shader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("compute_cubemap:{}", pass_ci.shader);
#endif
}

void CComputeCubemapPass::Build(const FPassBuildInfo& build_info)
{
	IComputePass::Build(build_info);
}

void CComputeCubemapPass::_Execute()
{
	auto& render_backend = m_pRenderInterface->GetRenderBackend();
	auto& render_graph = m_pRenderInterface->GetRenderGraph();
	auto& render_packet = m_pRenderInterface->GetRenderPacket();

	auto& write_resources = m_renderResources[ERenderResourceGroupType::eStorageTextureWrite];

	auto& shader = m_pRenderInterface->GetShaderObject(m_shader);

	for (const auto& resource_binding : write_resources)
	{
		auto& render_resource = render_graph->GetRenderResource(resource_binding.resource);
		auto info = render_backend->GetTextureView(render_resource.handle);

		for (u32 mip = 0u; mip < info.mipLevels; ++mip)
		{
			for (const auto& [binding_point, sesource_binding] : m_bakedBindings)
			{
				auto& binding_resource = render_graph->GetRenderResource(sesource_binding.resource);
				if (binding_resource.handle == render_resource.handle)
					shader->BindTexture(utl::murmur_hash_64(binding_point), binding_resource.handle, mip == 0u ? ~0u : mip);
				else
					shader->BindTexture(utl::murmur_hash_64(binding_point), binding_resource.handle);
			}

			f32 layer_roughness = utl::cast<f32>(mip) / utl::cast<f32>(info.mipLevels - 1u);

			auto& uniform = shader->GetUniformBlock(shader_bindings::compute_cubemap_data);
			if (uniform)
			{
				uniform->Set(shader_bindings::roughness_label, layer_roughness);
			}

			shader->FlushDescriptors();
			shader->FlushConstants(m_queue);
			shader->Bind(m_queue);
			shader->BindResources(m_queue);

			m_dispatchGroup.globalSizes.x = info.width;
			m_dispatchGroup.globalSizes.y = info.height;
			m_dispatchGroup.shaderHandle = shader->Handle();
			
			render_backend->Dispatch(m_queue, m_dispatchGroup, 0);
		}
	}
}