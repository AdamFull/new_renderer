#include "GeometryPass.h"

using namespace engine::render;

CGeometryPass::CGeometryPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CGeometryPass::Init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::Init(pass_ci);

	m_baseShaderName = pass_ci.shader;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("geometry:{}", m_baseShaderName);
#endif
}

void CGeometryPass::Build(const FPassBuildInfo& build_info)
{
	IRenderPass::Build(build_info);
}

void CGeometryPass::PostLoad()
{
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	m_cameraIndex = render_world->GetDefaultCameraID();
}

void CGeometryPass::_Render(const FRenderFrameInfo& frame_info)
{
	auto& resource_manager = m_pRenderInterface->GetResourceManager();
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->SetScissorRect(0, 0, frame_info.renderRect.size.x, frame_info.renderRect.size.y);

	auto& render_world = m_pRenderInterface->GetRenderWorld();
	const auto& world_stats = render_world->GetStats();

	const auto& draw_info = render_world->GetDrawCommandByType(ERenderGeometryCullerType::eScene).front();
	const auto& geometry_batches = render_world->GetGeometryBatches();

	auto begin_range = geometry_batches.begin() + draw_info.first;
	auto end_range = begin_range + draw_info.count;

	u64 drawcall_id{ draw_info.cullerID * world_stats.maxDrawCommands };
	IVertexHandler* last_vbo{ nullptr };
	for (auto it = begin_range; it != end_range; ++it)
	{
		const auto& batch = *it;

		bool was_vbo_rebinded{ false };
		auto& vbo = resource_manager->GetVertexHandler(batch.vboID);
		if (!last_vbo || vbo.get() != last_vbo)
		{
			last_vbo = vbo.get();
			last_vbo->Bind(m_queue);
			was_vbo_rebinded = true;
		}

		if (last_vbo)
		{
			// TODO: this code looks very ugly as i think
			// Create shader for vertex declaration if not exists
			auto& shader = GetShaderByVBO(last_vbo);

			if (was_vbo_rebinded)
			{
				shader->FlushDescriptors();
				shader->Bind(m_queue);
				shader->BindResources(m_queue);
			}

			auto& mesh_data = shader->GetConstantBlock(shader_bindings::mesh_data);
			if (mesh_data)
			{
				mesh_data->Set(shader_bindings::call_id_label, utl::cast<u32>(drawcall_id));
				mesh_data->Set(shader_bindings::batch_id_label, utl::cast<u32>(0));
			}

			// Bind vbo and flush descriptors, then bind and Draw

			shader->FlushConstants(m_queue);

			render_context->Draw(m_queue, batch.command);
			drawcall_id += draw_info.drawCount;
		}
	}
}

const std::unique_ptr<IShaderObject>& CGeometryPass::GetShaderByVBO(const IVertexHandler* vbo)
{
	auto vbo_hash = vbo->GetHash();
	auto& shader = m_shaders[vbo_hash];
	if (shader != INVALID_HANDLE)
	{
		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->GetDefinitions();
		auto front_face = m_bFlipViewport ? EShaderFrontFace::eCW : EShaderFrontFace::eCCW;

		shader = MakeShader(shader_name, EShaderCullModeFlagBits::eFront, front_face, false, EDepthCompareOp::eLessOrEqual, vbo);
		InitShaderResources(shader);
	}

	auto& resource_manager = m_pRenderInterface->GetResourceManager();
	return resource_manager->GetShaderObject(shader);
}