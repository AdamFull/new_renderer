#include "CascadedShadowMapPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CCascadedShadowMapPass::CCascadedShadowMapPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CCascadedShadowMapPass::Init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::Init(pass_ci);

	m_baseShaderName = pass_ci.shader;
	m_layerCount = cascade_shadow_map_cascade_count;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("csm:{}", m_baseShaderName);
#endif
}

void CCascadedShadowMapPass::Build(const FPassBuildInfo& build_info)
{
	IRenderPass::Build(build_info);
}

bool CCascadedShadowMapPass::CanRender() const
{
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	auto& world_stats = render_world->GetStats();
	return world_stats.cascadedShadowCount != 0;
}

void CCascadedShadowMapPass::PostLoad()
{
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	m_cameraIndex = render_world->GetCameraID(ERenderGeometryCullerType::eCascadedShadow);
}

void CCascadedShadowMapPass::_Render(const FRenderFrameInfo& frame_info)
{
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	auto& world_stats = render_world->GetStats();

	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->SetScissorRect(0, 0, frame_info.renderRect.size.x, frame_info.renderRect.size.y);

	IVertexHandler* last_vbo{ nullptr };

	const auto& draw_infos = render_world->GetDrawCommandByType(ERenderGeometryCullerType::eCascadedShadow);
	const auto& geometry_batches = render_world->GetGeometryBatches();
	
	for (u64 idx = 0ull; idx < draw_infos.size(); ++idx)
	{
		u64 current_layer = idx % 6ull;
		const auto& draw_info = draw_infos[idx];
		auto begin_range = geometry_batches.begin() + draw_info.first;
		auto end_range = begin_range + draw_info.count;

		u64 drawcall_id{ draw_info.cullerID * world_stats.maxDrawCommands };
		for (auto it = begin_range; it != end_range; ++it)
		{
			const auto& batch = *it;

			bool was_vbo_rebinded{ false };
			auto& vbo = m_pRenderInterface->GetVertexHandler(batch.vboID);
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
				for (u64 shadow_index = 0ull; shadow_index < world_stats.cascadedShadowCount; ++shadow_index)
				{
					shadow_data->Set(shader_bindings::shadow_index_label, utl::cast<u32>(shadow_index));
					shadow_data->Set(shader_bindings::layer_label, utl::cast<i32>(current_layer));
					shader->FlushConstants(m_queue);
					render_context->Draw(m_queue, batch.command);
				}
			}

			drawcall_id += draw_info.drawCount;
		}
	}
}

const std::unique_ptr<IShaderObject>& CCascadedShadowMapPass::GetShaderByVBO(const IVertexHandler* vbo)
{
	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	auto vbo_hash = vbo->GetHash();
	auto& shader = m_shaders[vbo_hash];
	if (!shader)
	{
		shader = std::make_unique<CShaderObject>(m_pRenderInterface);

		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->GetDefinitions();

		auto front_face = m_bFlipViewport ? EShaderFrontFace::eCW : EShaderFrontFace::eCCW;

		MakeShader(shader_name, shader, EShaderCullModeFlagBits::eFront, front_face, false, EDepthCompareOp::eLessOrEqual, vbo);
		InitShaderResources(shader);
	}

	return shader;
}