#include "ImGuiPass.h"

#include <imgui.h>

using namespace engine::render;

CImGuiPass::CImGuiPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
	ImGui::CreateContext();
}

CImGuiPass::~CImGuiPass()
{
	ImGui::DestroyContext();
}

void CImGuiPass::Init(const FRenderPassCreateInfo& pass_ci)
{
	auto& resource_manager = m_pRenderInterface->GetResourceManager();

	IRenderPass::Init(pass_ci);

	ImGuiIO& io = ImGui::GetIO();
	IMGUI_CHECKVERSION();
	IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

	io.BackendRendererUserData = (void*)this;
	io.BackendRendererName = "vulkan";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
	//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can Create multi-viewports on the Renderer side (optional)

	// At first create vertex/index handler
	FVertexHandlerCreateInfo vertex_handler_ci{};
	vertex_handler_ci.indexSize = sizeof(ImDrawIdx);
	vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 8ull, EVertexAttributeType::ePosition });
	vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 8ull, EVertexAttributeType::eTexCoord });
	vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 4ull, EVertexAttributeType::eColor });
	vertex_handler_ci.name = "imgui_vertices";
	m_vertexHandler = resource_manager->CreateVertexHandler("imgui_vertices", vertex_handler_ci);

	// Create font texture (just load) May be needs to implement texture interface
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	u64 upload_size = width * height * 4 * sizeof(char);

	FTextureCreateInfo create_info{};
	create_info.format = EApiDataFormat::eRGBA8_UNORM;
	create_info.typeFlags = ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eTransferable | ETextureCreateFlagBits::e2D;
	create_info.SetFixedSize2D(width, height);

	FTextureUploadInfo upload_info{};
	upload_info.data.insert(upload_info.data.end(), pixels, pixels + upload_size);

	auto& render_context = m_pRenderInterface->GetRenderContext();
	m_fontTexture = render_context->CreateTexture("imgui_font", create_info, std::move(upload_info));

	auto& vertex_handler = resource_manager->GetVertexHandler(m_vertexHandler);
	m_shader = MakeShader(pass_ci.shader, EShaderCullModeFlagBits::eNone, EShaderFrontFace::eCCW, true, EDepthCompareOp::eLessOrEqual, vertex_handler.get());
	InitShaderResources(m_shader);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("imgui:{}", pass_ci.shader);
#endif
}

void CImGuiPass::Build(const FPassBuildInfo& build_info)
{
	IRenderPass::Build(build_info);
}

void CImGuiPass::_Render(const FRenderFrameInfo& frame_info)
{
	auto& resource_manager = m_pRenderInterface->GetResourceManager();
	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto resolution = render_context->GetScreenRenderRect();

	auto& vertex_handler = resource_manager->GetVertexHandler(m_vertexHandler);
	auto& shader = resource_manager->GetShaderObject(m_shader);

	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2(resolution.size.x, resolution.size.y);
	io.DeltaTime = frame_info.deltaTime;

	ImGui::NewFrame();

	static bool show_demo_window{ true };
	ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Render();

	// Render ingui
	{
		ImDrawData* draw_data = ImGui::GetDrawData();

		auto resolution = render_context->GetScreenRenderRect();

		u32 last_vertex{ 0u };
		u32 last_index{ 0u };

		// Update vertices
		if (draw_data->TotalVtxCount > 0)
		{
			// Upload vertex/index data into a single contiguous GPU buffer
			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				vertex_handler->AddVertices(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size, sizeof(ImDrawVert));
				vertex_handler->AddIndices(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size, sizeof(ImDrawIdx));

				last_vertex += cmd_list->VtxBuffer.Size;
				last_index += cmd_list->IdxBuffer.Size;
			}
		}

		// Upload vertices
		vertex_handler->Allocate();
		//render_context->UploadBuffers();

		// Bind pipeline:
		shader->Bind(m_queue);

		// Bind Vertex And Index Buffer:
		if (draw_data->TotalVtxCount > 0)
			vertex_handler->Bind(m_queue);

		// Setup viewport:
		render_context->SetViewport(resolution.size.x, resolution.size.y, false);

		// Setup scale and translation:
		// Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
		{
			auto& constants = shader->GetConstantBlock(utl::fnv1a_64_hash_cstr("constants"));
			if (constants)
			{
				glm::vec2 scale;
				scale.x = 2.f / draw_data->DisplaySize.x;
				scale.y = 2.f / draw_data->DisplaySize.y;

				constants->Set(utl::fnv1a_64_hash_cstr("scale"), scale);

				glm::vec2 translate;
				translate.x = -1.0f - draw_data->DisplayPos.x * scale.x;
				translate.y = -1.0f - draw_data->DisplayPos.y * scale.y;

				constants->Set(utl::fnv1a_64_hash_cstr("translate"), translate);

				shader->FlushConstants(m_queue);
			}
		}

		u64 last_bound_resource{ INVALID_HANDLE };

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		// (Because we merged all buffers into a single one, we maintain our own offset into them)
		i32 global_vtx_offset = 0;
		i32 global_idx_offset = 0;
		for (i32 n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			for (i32 cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback != nullptr)
				{
					// User callback, registered via ImDrawList::AddCallback()
					// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
					//if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					//	ImGui_ImplVulkan_SetupRenderState(draw_data, pipeline, command_buffer, rb, fb_width, fb_height);
					//else
						pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					// Project scissor/clipping rectangles into framebuffer space
					ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
					ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

					// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
					if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
					if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
					if (clip_max.x > resolution.size.x) { clip_max.x = resolution.size.x; }
					if (clip_max.y > resolution.size.y) { clip_max.y = resolution.size.y; }
					if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
						continue;

					// Apply scissor/clipping rectangle
					render_context->SetScissorRect((int32_t)(clip_min.x), (int32_t)(clip_min.y), (uint32_t)(clip_max.x - clip_min.x), (uint32_t)(clip_max.y - clip_min.y));

					// Bind DescriptorSet with font or user texture
					u64 attachment{ (u64)pcmd->TextureId };
					if (sizeof(ImTextureID) < sizeof(ImU64))
					{
						// We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a flaky check that other textures haven't been used.
						IM_ASSERT(pcmd->TextureId == (ImTextureID)m_fontTexture);
						attachment = m_fontTexture;
					}

					if(!pcmd->TextureId)
						attachment = m_fontTexture;

					if (attachment != last_bound_resource)
					{
						shader->BindTexture(utl::fnv1a_64_hash_cstr("fontSampler"), attachment);
						shader->FlushDescriptors();
						shader->BindResources(m_queue);
						last_bound_resource = attachment;
					}

					FGeometryDrawInfo draw_info{};
					draw_info.lastVertex = last_vertex;
					draw_info.lastIndex = last_index;

					auto& range = draw_info.ranges.emplace_back();
					range.beginIndex = pcmd->IdxOffset + global_idx_offset;
					range.indexCount = pcmd->ElemCount;
					range.beginVertex = pcmd->VtxOffset + global_vtx_offset;

					// Draw
					render_context->Draw(m_queue, draw_info);
				}
			}
			global_idx_offset += cmd_list->IdxBuffer.Size;
			global_vtx_offset += cmd_list->VtxBuffer.Size;
		}

		// Note: at this point both vkCmdSetViewport() and vkCmdSetScissor() have been called.
		// Our last values will leak into user/application rendering IF:
		// - Your app uses a pipeline with VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR dynamic state
		// - And you forgot to call vkCmdSetViewport() and vkCmdSetScissor() yourself to explicitly set that state.
		// If you use VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR you are responsible for setting the values before rendering.
		// In theory we should aim to backup/restore those values but I am not sure this is possible.
		// We perform a call to vkCmdSetScissor() to set back a full viewport which is likely to fix things for 99% users but technically this is not perfect. (See github #4644)
		render_context->SetScissorRect(0, 0, (u32)resolution.size.x, (u32)resolution.size.y);
	}
}