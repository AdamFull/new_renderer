#include "FullscreenPass.h"

#include <engine/render/ShaderObject.h>

#include <format>

using namespace engine::render;

CFullscreenPass::CFullscreenPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CFullscreenPass::Init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::Init(pass_ci);

	auto& render_graph = m_pRenderInterface->GetRenderGraph();

	FGraphicsShaderCreateInfo shader_ci{};
	shader_ci.frontFace = EShaderFrontFace::eCCW;
	shader_ci.cullMode = pass_ci.flipViewport ? EShaderCullModeFlagBits::eBack : EShaderCullModeFlagBits::eFront;
	shader_ci.primitiveTopology = EVertexPrimitiveTopology::eTriangleList;

	m_shader = MakeShader(pass_ci.shader, shader_ci);
	InitShaderResources(m_shader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("fullscreen:{}", pass_ci.shader);
#endif
}

void CFullscreenPass::Build(const FPassBuildInfo& build_info)
{
	IRenderPass::Build(build_info);
}

void CFullscreenPass::Update(f32 dt)
{
	m_deltaTime = dt;
}

void CFullscreenPass::_Render()
{
	auto& render_backend = m_pRenderInterface->GetRenderBackend();

	auto& shader = m_pRenderInterface->GetShaderObject(m_shader);
	BindShaderResources(m_shader, m_deltaTime);

	auto resolution = render_backend->GetScreenRenderRect();
	render_backend->SetViewport(resolution.size.x, resolution.size.y, m_bFlipViewport);
	render_backend->SetScissorRect(0, 0, resolution.size.x, resolution.size.y);

	shader->FlushDescriptors();
	shader->FlushConstants(m_queue);
	shader->Bind(m_queue);
	shader->BindResources(m_queue);

	FGeometryDrawInfo draw_info{};
	auto& range = draw_info.ranges.emplace_back();
	range.beginIndex = 0ull;
	range.indexCount = 0ull;
	range.beginVertex = 0ull;
	range.vertexCount = 3ull;

	render_backend->Draw(m_queue, draw_info);
}