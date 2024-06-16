#include "DebugDrawPass.h"

#include <engine/render/ShaderObject.h>
#include <engine/render/VertexHandler.h>

#include <DebugDraw/DebugDraw.h>

using namespace engine;
using namespace engine::render;

CDebugDrawPass::CDebugDrawPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

CDebugDrawPass::~CDebugDrawPass()
{
}

void CDebugDrawPass::Init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::Init(pass_ci);

	DebugDraw::Init();

	// At first create vertex/index handler
	FVertexHandlerCreateInfo vertex_handler_ci{};
	vertex_handler_ci.indexSize = sizeof(u16);
	vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 12ull, EVertexAttributeType::ePosition });
	vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 8ull, EVertexAttributeType::eTexCoord });
	vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 4ull, EVertexAttributeType::eColor });
	vertex_handler_ci.name = "debug_vertices";
	m_vertexHandler = m_pRenderInterface->CreateVertexHandler("debug_vertices", vertex_handler_ci);

	// Make RHI shader object
	auto& vertex_handler = m_pRenderInterface->GetVertexHandler(m_vertexHandler);
	{
		FGraphicsShaderCreateInfo shader_ci{};
		shader_ci.frontFace = EShaderFrontFace::eCCW;
		shader_ci.cullMode = EShaderCullModeFlagBits::eNone;
		shader_ci.primitiveTopology = EVertexPrimitiveTopology::ePointList;

		//m_shaders[(u32)DebugGeometryType::ePoints] = MakeShader(pass_ci.shader, shader_ci, vertex_handler.get());
		//InitShaderResources(m_shaders[(u32)DebugGeometryType::ePoints]);
	}

	{
		FGraphicsShaderCreateInfo shader_ci{};
		shader_ci.frontFace = EShaderFrontFace::eCCW;
		shader_ci.cullMode = EShaderCullModeFlagBits::eNone;
		shader_ci.primitiveTopology = EVertexPrimitiveTopology::eLineList;

		m_shaders[(u32)DebugGeometryType::eLines] = MakeShader(pass_ci.shader, shader_ci, vertex_handler.get());
		InitShaderResources(m_shaders[(u32)DebugGeometryType::eLines]);
	}

	{
		FGraphicsShaderCreateInfo shader_ci{};
		shader_ci.frontFace = EShaderFrontFace::eCCW;
		shader_ci.cullMode = EShaderCullModeFlagBits::eNone;
		shader_ci.primitiveTopology = EVertexPrimitiveTopology::eTriangleList;

		m_shaders[(u32)DebugGeometryType::eTriangles] = MakeShader(pass_ci.shader, shader_ci, vertex_handler.get());
		InitShaderResources(m_shaders[(u32)DebugGeometryType::eTriangles]);
	}

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("debug:{}", pass_ci.shader);
#endif
}

void CDebugDrawPass::Build(const FPassBuildInfo& build_info)
{
	IRenderPass::Build(build_info);
}

void CDebugDrawPass::Update(f32 dt)
{
	m_lastVertex = 0;
	m_lastIndex = 0;

	const auto& vertices = DebugDraw::GetVertices();
	const auto& indices = DebugDraw::GetIndices();
	auto& vertex_handler = m_pRenderInterface->GetVertexHandler(m_vertexHandler);

	// Update vertices
	if (vertices.size())
	{
		// Upload vertex/index data into a single contiguous GPU buffer
		vertex_handler->AddVertices(vertices.data(), vertices.size(), sizeof(Vertex));
		vertex_handler->AddIndices(indices.data(), indices.size(), sizeof(u16));

		m_lastVertex += vertices.size();
		m_lastIndex += indices.size();

		// Upload vertices
		vertex_handler->Allocate();
	}
}

void CDebugDrawPass::_Render()
{
	static const std::array<DebugGeometryType, 3> draw_types{ DebugGeometryType::ePoints, DebugGeometryType::eLines, DebugGeometryType::eTriangles };

	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto resolution = render_context->GetScreenRenderRect();

	// At first update vertices if needed
	auto& vertex_handler = m_pRenderInterface->GetVertexHandler(m_vertexHandler);

	// Bind Vertex And Index Buffer:
	const auto& vertices = DebugDraw::GetVertices();
	if (vertices.size())
		vertex_handler->Bind(m_queue);

	// Setup viewport:
	render_context->SetViewport(resolution.size.x, resolution.size.y, false);

	for (const auto& draw_type : draw_types)
	{
		const auto& draw_commands = DebugDraw::GetDrawCommands(draw_type);
		if (draw_commands.empty())
			continue;

		const auto& shader_id = m_shaders[(u32)draw_type];
		auto& shader = m_pRenderInterface->GetShaderObject(shader_id);
		shader->Bind(m_queue);

		// TODO: add font texture support
		shader->FlushDescriptors();
		shader->BindResources(m_queue);

		for (const auto& command : draw_commands)
		{
			// Project scissor/clipping rectangles into framebuffer space
			glm::vec2 clip_min = { command.clip_rect.x, command.clip_rect.y };
			glm::vec2 clip_max = { command.clip_rect.z, command.clip_rect.w };

			// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
			if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
			if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
			if (clip_max.x > resolution.size.x) { clip_max.x = resolution.size.x; }
			if (clip_max.y > resolution.size.y) { clip_max.y = resolution.size.y; }
			if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
				continue;

			// Apply scissor/clipping rectangle
			//render_context->SetScissorRect((int32_t)(clip_min.x), (int32_t)(clip_min.y), (uint32_t)(clip_max.x - clip_min.x), (uint32_t)(clip_max.y - clip_min.y));
			render_context->SetScissorRect(0, 0, resolution.size.x, resolution.size.y);

			FGeometryDrawInfo draw_info{};
			draw_info.lastVertex = m_lastVertex;
			draw_info.lastIndex = m_lastIndex;

			auto& range = draw_info.ranges.emplace_back();
			range.beginIndex = command.index_offset;// +global_idx_offset;
			range.indexCount = command.index_count;
			range.beginVertex = command.vertex_offset;// +global_vtx_offset;

			// Draw
			render_context->Draw(m_queue, draw_info);
		}
	}
}