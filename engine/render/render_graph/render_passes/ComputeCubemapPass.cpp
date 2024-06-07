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

	auto& out_targets = m_renderResources[ERenderResourceGroupType::eStorageTextureWrite];
	for (const auto& resource_binding : out_targets)
	{
		auto& texture = m_pRenderInterface->GetTextureObject(resource_binding.resource);

		u32  mip_count{};
		texture->GetMipLevelCount(&mip_count);
		calculated_usage += mip_count;
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

void CComputeCubemapPass::_Execute(const FComputeFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& render_graph = m_pRenderInterface->GetRenderGraph();
	auto& render_packet = m_pRenderInterface->GetRenderPacket();

	auto& write_resources = m_renderResources[ERenderResourceGroupType::eStorageTextureWrite];

	auto& shader = m_pRenderInterface->GetShaderObject(m_shader);

	for (const auto& resource_binding : write_resources)
	{
		u32 width, height, mip_count;
		auto& texture = m_pRenderInterface->GetTextureObject(resource_binding.resource);
		texture->GetDimensions(&width, &height);
		texture->GetMipLevelCount(&mip_count);
		for (u32 mip = 0u; mip < mip_count; ++mip)
		{
			for (const auto& [binding_point, sesource_binding] : m_bakedBindings)
			{
				if (sesource_binding.resource == resource_binding.resource)
					shader->BindTexture(utl::fnv1a_64_hash_cstr(binding_point.c_str()), texture->Handle(), mip == 0u ? ~0u : mip);
				else
					shader->BindTexture(utl::fnv1a_64_hash_cstr(binding_point.c_str()), m_pRenderInterface->GetTextureObject(sesource_binding.resource)->Handle());
			}

			f32 layer_roughness = utl::cast<f32>(mip) / utl::cast<f32>(mip_count - 1u);

			auto& uniform = shader->GetUniformBlock(shader_bindings::compute_cubemap_data);
			if (uniform)
			{
				uniform->Set(shader_bindings::roughness_label, layer_roughness);
			}

			shader->FlushDescriptors();
			shader->FlushConstants(m_queue);
			shader->Bind(m_queue);
			shader->BindResources(m_queue);

			auto& dispatch_info = m_vDispatchGroups[0];
			dispatch_info.globalSizes.x = width;
			dispatch_info.globalSizes.y = height;
			dispatch_info.shaderHandle = shader->Handle();
			
			render_context->Dispatch(m_queue, dispatch_info, 0);
		}
	}
}