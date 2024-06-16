#include "DirectionalShadowMapPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CDirectionalShadowMapPass::CDirectionalShadowMapPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CDirectionalShadowMapPass::Init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::Init(pass_ci);

	m_baseShaderName = pass_ci.shader;
	m_layerCount = max_point_light_shadow_count;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("dsm:{}", m_baseShaderName);
#endif
}

void CDirectionalShadowMapPass::Build(const FPassBuildInfo& build_info)
{
	IRenderPass::Build(build_info);
}

bool CDirectionalShadowMapPass::CanRender() const
{
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	auto& world_stats = render_world->GetStats();
	return world_stats.spotShadowCount != 0;
}

void CDirectionalShadowMapPass::PostLoad()
{
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	m_cameraIndex = render_world->GetCameraID(ERenderGeometryCullerType::eSpotShadow);
}

void CDirectionalShadowMapPass::_Render()
{
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	auto& world_stats = render_world->GetStats();

	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->SetScissorRect(0, 0, m_renderArea.size.x, m_renderArea.size.y);

	IVertexHandler* last_vbo{ nullptr };

	const auto& draw_infos = render_world->GetDrawCommandByType(ERenderGeometryCullerType::eSpotShadow);
	const auto& geometry_batches = render_world->GetGeometryBatches();

	for (u64 idx = 0ull; idx < draw_infos.size(); ++idx)
	{
		const auto& draw_info = draw_infos[idx];
		auto begin_range = geometry_batches.begin() + draw_info.first;
		auto end_range = begin_range + draw_info.count;

		u64 drawcall_id{ draw_info.cullerID * world_stats.maxDrawCommands };
		for (auto it = begin_range; it != end_range; ++it)
		{
			const auto& batch = *it;

			bool was_vbo_rebinded{ false };
			auto& vbo = m_pRenderInterface->GetVertexHandler(FRHIHandle(batch.vboID));
			if (!last_vbo || vbo.get() != last_vbo)
			{
				last_vbo = vbo.get();
				last_vbo->Bind(m_queue);
				was_vbo_rebinded = true;
			}

			if (last_vbo)
			{
				auto& shader = GetShaderByVBO(last_vbo);
				if (was_vbo_rebinded)
				{
					shader->FlushDescriptors();
					shader->Bind(m_queue);
					shader->BindResources(m_queue);
				}

				auto& shadow_data = shader->GetConstantBlock(shader_bindings::shader_data);
				shadow_data->Set(shader_bindings::call_id_label, utl::cast<u32>(drawcall_id));
				shadow_data->Set(shader_bindings::shadow_index_label, utl::cast<u32>(idx));
				shadow_data->Set(shader_bindings::layer_label, utl::cast<i32>(idx));

				shader->FlushConstants(m_queue);
				render_context->Draw(m_queue, batch.command);
			}

			drawcall_id += draw_info.drawCount;
		}
	}
}

const std::unique_ptr<IShaderObject>& CDirectionalShadowMapPass::GetShaderByVBO(const IVertexHandler* vbo)
{
	auto vbo_hash = vbo->GetHash();
	auto& shader = m_shaders[vbo_hash];
	if (!shader)
	{
		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->GetDefinitions();

		FGraphicsShaderCreateInfo shader_ci{};
		shader_ci.frontFace = m_bFlipViewport ? EShaderFrontFace::eCW : EShaderFrontFace::eCCW;
		shader_ci.cullMode = EShaderCullModeFlagBits::eFront;
		shader_ci.primitiveTopology = EVertexPrimitiveTopology::eTriangleList;

		shader = MakeShader(shader_name, shader_ci, vbo);
		InitShaderResources(shader);
	}

	return m_pRenderInterface->GetShaderObject(shader);
}