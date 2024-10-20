#include "ComputeCullingPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CComputeCullingPass::CComputeCullingPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputeCullingPass::Init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::Init(pass_ci);

	m_baseShaderName = pass_ci.shader;

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("culling:{}", m_baseShaderName);
#endif
}

void CComputeCullingPass::Build(const FPassBuildInfo& build_info)
{
	IComputePass::Build(build_info);
}

void CComputeCullingPass::Update(f32 dt)
{
	m_deltaTime = dt;
}

void CComputeCullingPass::_Execute()
{
	auto& render_backend = m_pRenderInterface->GetRenderBackend();
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	auto& world_stats = render_world->GetStats();

	std::unordered_set<FRHIHandle> vbos{};

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
					buffers->Set(shader_bindings::last_index_label, utl::cast<u32>(index_buffer->Count()));
					buffers->Set(shader_bindings::last_vertex_label, utl::cast<u32>(vertex_buffer->Count()));
				}

				shader->Bind(m_queue);
				shader->BindResources(m_queue);
			}

			shader->FlushConstants(m_queue);

			FDispatch2Info dispatch_info{};
			dispatch_info.divideByLocals = true;
			dispatch_info.globalSizes.x = world_stats.drawCommandCount;
			dispatch_info.globalSizes.y = 1;
			dispatch_info.globalSizes.z = 1;
			dispatch_info.shaderHandle = shader->Handle();

			render_backend->Dispatch(m_queue, dispatch_info, 0);
		}
	}
}

const std::unique_ptr<IShaderObject>& CComputeCullingPass::GetShaderByVBO(const IVertexHandler* vbo)
{
	auto& shader = m_shaders[vbo->GetHash()];
	if (!shader)
	{
		// Generate shader name
		auto shader_name = m_baseShaderName + vbo->GetDefinitions();
		auto& render_packet = m_pRenderInterface->GetRenderPacket();

		FComputeShaderCreateInfo shader_ci{};
		shader = m_pRenderInterface->CreateShaderObject(shader_name, shader_ci);

		InitShaderResources(shader);
	}

	return m_pRenderInterface->GetShaderObject(shader);
}