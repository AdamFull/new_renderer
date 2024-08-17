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
	ImDbg::DestroyContext();
}

void CDebugDrawPass::Init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::Init(pass_ci);

	ImDbg::CreateContext();

	u8* pixels{ nullptr };
	i32 width, height, stride;
	ImDbg::GetTexDataAsRGBA32(0, &pixels, &width, &height, &stride);
	u64 upload_size = width * height * stride;

	FTextureCreateInfo font_ci{};
	font_ci.format = EApiDataFormat::eRGBA8_UNORM;
	font_ci.flags = ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eTransferable | ETextureCreateFlagBits::e2D;
	font_ci.SetFixedSize2D(width, height);

	FTextureUploadInfo upload_info{};
	upload_info.data.insert(upload_info.data.end(), pixels, pixels + upload_size);

	auto& render_context = m_pRenderInterface->GetRenderContext();
	m_fontTexture = m_pRenderInterface->CreateTextureObject("default_debug_font", font_ci, std::move(upload_info));

	// Init 2d part
	{
		FVertexHandlerCreateInfo vertex_handler_ci{};
		vertex_handler_ci.indexSize = sizeof(u16);
		vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 12ull, EVertexAttributeType::ePosition });
		vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 8ull, EVertexAttributeType::eTexCoord });
		vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 4ull, EVertexAttributeType::eColor });
		vertex_handler_ci.name = "debug_vertices_2d";
		m_vertexHandler = m_pRenderInterface->CreateVertexHandler("debug_vertices_2d", vertex_handler_ci);

		auto& vertex_handler = m_pRenderInterface->GetVertexHandler(m_vertexHandler);
	
		FGraphicsShaderCreateInfo shader_ci{};
		shader_ci.frontFace = EShaderFrontFace::eCCW;
		shader_ci.cullMode = EShaderCullModeFlagBits::eNone;
		shader_ci.primitiveTopology = EVertexPrimitiveTopology::eTriangleList;

		m_shader = MakeShader(pass_ci.shader, shader_ci, vertex_handler.get());
		InitShaderResources(m_shader);
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
	auto* draw_data = ImDbg::GetDrawData();

	m_lastVertex = 0;
	m_lastIndex = 0;

	// Update vertices
	if (draw_data->totalVertexCount > 0)
	{
		auto& vertex_handler = m_pRenderInterface->GetVertexHandler(m_vertexHandler);
		for (const auto& draw_list : draw_data->drawLists)
		{
			// Upload vertex/index data into a single contiguous GPU buffer
			vertex_handler->AddVertices(draw_list.vertices.data(), draw_list.vertices.size(), sizeof(ImDbgVertex));
			vertex_handler->AddIndices(draw_list.indices.data(), draw_list.indices.size(), sizeof(ImDbgIndex));

			m_lastVertex += draw_list.vertices.size();
			m_lastIndex += draw_list.indices.size();
		}

		// Upload vertices
		vertex_handler->Allocate();
	}
}

void CDebugDrawPass::Render2D()
{

}

void CDebugDrawPass::Render3D()
{

}

void CDebugDrawPass::_Render()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto resolution = render_context->GetScreenRenderRect();

	ImDbgIO* io = ImDbg::GetIO();

	{
		auto* draw_data = ImDbg::GetDrawData();

		// At first update vertices if needed
		auto& vertex_handler = m_pRenderInterface->GetVertexHandler(m_vertexHandler);
		auto& shader = m_pRenderInterface->GetShaderObject(m_shader);

		// Bind Vertex And Index Buffer:
		if (draw_data->totalVertexCount > 0)
			vertex_handler->Bind(m_queue);

		// Setup viewport:
		render_context->SetViewport(resolution.size.x, resolution.size.y, false);

		u32 global_idx_offset{ 0u };
		u32 global_vtx_offset{ 0u };

		for (const auto& draw_list : draw_data->drawLists)
		{
			const auto& draw_commands = draw_list.drawCommands;
			if (draw_commands.empty())
				continue;

			shader->Bind(m_queue);

			EVertexPrimitiveTopology topology{ EVertexPrimitiveTopology::eLineList };
			if (draw_list.type == ImDbgGeometryType::ePoints)
				topology = EVertexPrimitiveTopology::ePointList;
			else if (draw_list.type == ImDbgGeometryType::eLines)
				topology = EVertexPrimitiveTopology::eLineList;
			else if (draw_list.type == ImDbgGeometryType::eTriangles)
				topology = EVertexPrimitiveTopology::eTriangleList;

			render_context->SetPrimitiveTopology(EVertexPrimitiveTopology::eTriangleList);

			auto& constants = shader->GetConstantBlock(utl::murmur_hash_64("constants"));
			if (constants)
			{
				glm::vec2 scale;
				scale.x = 2.f / io->screenSize.x;
				scale.y = 2.f / io->screenSize.y;

				constants->Set(utl::murmur_hash_64("scale"), scale);

				glm::vec2 translate;
				translate.x = -1.0f - 0.f * scale.x;
				translate.y = -1.0f - 0.f * scale.y;

				constants->Set(utl::murmur_hash_64("translate"), translate);

				shader->FlushConstants(m_queue);
			}

			shader->BindTexture(utl::murmur_hash_64("font_sampler"), m_pRenderInterface->GetTextureObject(m_fontTexture)->Handle());

			// TODO: add font texture support
			shader->FlushDescriptors();
			shader->BindResources(m_queue);

			for (const auto& command : draw_commands)
			{
				// Project scissor/clipping rectangles into framebuffer space
				glm::vec2 clip_min = { command.clipRect.x, command.clipRect.y };
				glm::vec2 clip_max = { command.clipRect.z, command.clipRect.w };

				// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
				if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
				if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
				if (clip_max.x > resolution.size.x) { clip_max.x = resolution.size.x; }
				if (clip_max.y > resolution.size.y) { clip_max.y = resolution.size.y; }
				if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
				{
					clip_min.x = clip_min.y = 0.0f;
					clip_max.x = resolution.size.x;
					clip_max.y = resolution.size.y;
				}

				// Apply scissor/clipping rectangle
				render_context->SetScissorRect((int32_t)(clip_min.x), (int32_t)(clip_min.y), (uint32_t)(clip_max.x - clip_min.x), (uint32_t)(clip_max.y - clip_min.y));
				//render_context->SetScissorRect(0, 0, resolution.size.x, resolution.size.y);

				FGeometryDrawInfo draw_info{};
				draw_info.lastVertex = m_lastVertex;
				draw_info.lastIndex = m_lastIndex;

				auto& range = draw_info.ranges.emplace_back();
				range.beginIndex = command.indexOffset + global_idx_offset;
				range.indexCount = command.indexCount;
				range.beginVertex = command.vertexOffset + global_vtx_offset;

				// Draw
				render_context->Draw(m_queue, draw_info);
			}

			global_idx_offset += draw_list.indices.size();
			global_vtx_offset += draw_list.vertices.size();
		}
	}
}