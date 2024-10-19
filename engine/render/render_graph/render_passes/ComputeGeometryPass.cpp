#include "ComputeGeometryPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CComputeGeometryPass::CComputeGeometryPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputeGeometryPass::Init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::Init(pass_ci);

	m_baseShaderName = pass_ci.shader;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("compute_geometry:{}", m_baseShaderName);
#endif
}

void CComputeGeometryPass::Build(const FPassBuildInfo& build_info)
{
	IComputePass::Build(build_info);
}

void CComputeGeometryPass::Update(f32 dt)
{
	m_deltaTime = dt;
}

void CComputeGeometryPass::_Execute()
{
	std::unordered_set<FRHIHandle> vbos{};

	auto& render_backend = m_pRenderInterface->GetRenderBackend();
	auto& render_world = m_pRenderInterface->GetRenderWorld();


	const auto& geometry_batches = render_world->GetGeometryBatches();
	for (const auto& batch : geometry_batches)
		vbos.emplace(batch.vboID);

	IVertexHandler* last_vbo{ nullptr };

	for (auto& vboId : vbos)
	{
		bool was_vbo_rebinded{ false };
		auto& vbo = m_pRenderInterface->GetVertexHandler(vboId);
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
				BindShaderResources(shader, m_deltaTime);
				if (auto& buffers = shader->GetUniformBlock(shader_bindings::geometry_buffers); buffers)
				{
					auto& vertex_buffer = last_vbo->GetGeometryBuffer(EGeometryBufferIndices::eVertexBuffer);
					auto& index_buffer = last_vbo->GetGeometryBuffer(EGeometryBufferIndices::eIndexBuffer);

					buffers->Set(shader_bindings::vb_label, vertex_buffer->Buffer()->GetDescription());
					buffers->Set(shader_bindings::ib_label, index_buffer->Buffer()->GetDescription());
					buffers->Set(shader_bindings::mb_label, last_vbo->GetGeometryBuffer(EGeometryBufferIndices::eMeshletBuffer)->Buffer()->GetDescription());
					buffers->Set(shader_bindings::vrb_label, last_vbo->GetGeometryBuffer(EGeometryBufferIndices::eVertexRemapBuffer)->Buffer()->GetDescription());
					buffers->Set(shader_bindings::bb_label, last_vbo->GetGeometryBuffer(EGeometryBufferIndices::eBoundsBuffer)->Buffer()->GetDescription());
					buffers->Set(shader_bindings::tb_label, last_vbo->GetGeometryBuffer(EGeometryBufferIndices::eTriangleBuffer)->Buffer()->GetDescription());
					buffers->Set(shader_bindings::lb_label, last_vbo->GetGeometryBuffer(EGeometryBufferIndices::eLodBuffer)->Buffer()->GetDescription());
				}

				shader->Bind(m_queue);
				shader->BindResources(m_queue);
			}

			auto& mesh_data = shader->GetConstantBlock(shader_bindings::mesh_data);
			if (mesh_data)
			{
				mesh_data->Set(shader_bindings::vbo_id_label, utl::cast<u32>(vboId.Get()));
			}

			shader->FlushConstants(m_queue);

			m_dispatchGroup.shaderHandle = shader->Handle();
			render_backend->Dispatch(m_queue, m_dispatchGroup, 0);
		}
	}
}

const std::unique_ptr<IShaderObject>& CComputeGeometryPass::GetShaderByVBO(const IVertexHandler* vbo)
{
	auto& shader = m_shaders[vbo->GetHash()];
	if (!shader)
	{
		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->GetDefinitions();
		auto& render_packet = m_pRenderInterface->GetRenderPacket();

		FComputeShaderCreateInfo shader_ci{};
		shader_ci.usageCount = std::max(render_packet->stats.materialCount, render_packet->stats.primitiveCount);
		shader = m_pRenderInterface->CreateShaderObject(shader_name, shader_ci);

		InitShaderResources(shader);
	}

	return m_pRenderInterface->GetShaderObject(shader);
}