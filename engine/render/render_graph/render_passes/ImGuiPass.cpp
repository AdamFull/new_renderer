#include "ImGuiPass.h"

#include <engine/render/ShaderObject.h>
#include <engine/render/VertexHandler.h>
#include <engine/platform/window/WindowCallback.h>

#include <format>

#include <imgui.h>

using namespace engine;
using namespace engine::render;

ImGuiKey key_code_to_im_gui_key(EKeyCode code)
{
	switch (code)
	{
	case EKeyCode::eKeyTab: return ImGuiKey_Tab;
	case EKeyCode::eKeyLeft: return ImGuiKey_LeftArrow;
	case EKeyCode::eKeyRight: return ImGuiKey_RightArrow;
	case EKeyCode::eKeyUp: return ImGuiKey_UpArrow;
	case EKeyCode::eKeyDown: return ImGuiKey_DownArrow;
	case EKeyCode::eKeyPageUp: return ImGuiKey_PageUp;
	case EKeyCode::eKeyPageDown: return ImGuiKey_PageDown;
	case EKeyCode::eKeyHome: return ImGuiKey_Home;
	case EKeyCode::eKeyEnd: return ImGuiKey_End;
	case EKeyCode::eKeyInsert: return ImGuiKey_Insert;
	case EKeyCode::eKeyDelete: return ImGuiKey_Delete;
	case EKeyCode::eKeyBackspace: return ImGuiKey_Backspace;
	case EKeyCode::eKeySpace: return ImGuiKey_Space;
	case EKeyCode::eKeyReturn: return ImGuiKey_Enter;
	case EKeyCode::eKeyEscape: return ImGuiKey_Escape;
	case EKeyCode::eKeyApostrophe: return ImGuiKey_Apostrophe;
	case EKeyCode::eKeyComma: return ImGuiKey_Comma;
	case EKeyCode::eKeyMinus: return ImGuiKey_Minus;
	case EKeyCode::eKeyPeriod: return ImGuiKey_Period;
	case EKeyCode::eKeySlash: return ImGuiKey_Slash;
	case EKeyCode::eKeySemicolon: return ImGuiKey_Semicolon;
	case EKeyCode::eKeyEquals: return ImGuiKey_Equal;
	case EKeyCode::eKeyLeftBracket: return ImGuiKey_LeftBracket;
	case EKeyCode::eKeyBackslash: return ImGuiKey_Backslash;
	case EKeyCode::eKeyRightBracket: return ImGuiKey_RightBracket;
		//case GLFW_KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
	case EKeyCode::eKeyCapslock: return ImGuiKey_CapsLock;
	case EKeyCode::eKeyScrollLock: return ImGuiKey_ScrollLock;
		//case GLFW_KEY_NUM_LOCK: return ImGuiKey_NumLock;
	case EKeyCode::eKeyPrintscreen: return ImGuiKey_PrintScreen;
	case EKeyCode::eKeyPause: return ImGuiKey_Pause;
		//case GLFW_KEY_KP_0: return ImGuiKey_Keypad0;
		//case GLFW_KEY_KP_1: return ImGuiKey_Keypad1;
		//case GLFW_KEY_KP_2: return ImGuiKey_Keypad2;
		//case GLFW_KEY_KP_3: return ImGuiKey_Keypad3;
		//case GLFW_KEY_KP_4: return ImGuiKey_Keypad4;
		//case GLFW_KEY_KP_5: return ImGuiKey_Keypad5;
		//case GLFW_KEY_KP_6: return ImGuiKey_Keypad6;
		//case GLFW_KEY_KP_7: return ImGuiKey_Keypad7;
		//case GLFW_KEY_KP_8: return ImGuiKey_Keypad8;
		//case GLFW_KEY_KP_9: return ImGuiKey_Keypad9;
		//case GLFW_KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
		//case GLFW_KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
		//case GLFW_KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
		//case GLFW_KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
		//case GLFW_KEY_KP_ADD: return ImGuiKey_KeypadAdd;
		//case GLFW_KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
		//case GLFW_KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
	case EKeyCode::eKeyLShift: return ImGuiKey_LeftShift;
	case EKeyCode::eKeyLCtrl: return ImGuiKey_LeftCtrl;
	case EKeyCode::eKeyLAlt: return ImGuiKey_LeftAlt;
		//case GLFW_KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
	case EKeyCode::eKeyRShift: return ImGuiKey_RightShift;
	case EKeyCode::eKeyRCtrl: return ImGuiKey_RightCtrl;
	case EKeyCode::eKeyRAlt: return ImGuiKey_RightAlt;
		//case GLFW_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
		//case GLFW_KEY_MENU: return ImGuiKey_Menu;
	case EKeyCode::eKey0: return ImGuiKey_0;
	case EKeyCode::eKey1: return ImGuiKey_1;
	case EKeyCode::eKey2: return ImGuiKey_2;
	case EKeyCode::eKey3: return ImGuiKey_3;
	case EKeyCode::eKey4: return ImGuiKey_4;
	case EKeyCode::eKey5: return ImGuiKey_5;
	case EKeyCode::eKey6: return ImGuiKey_6;
	case EKeyCode::eKey7: return ImGuiKey_7;
	case EKeyCode::eKey8: return ImGuiKey_8;
	case EKeyCode::eKey9: return ImGuiKey_9;
	case EKeyCode::eKeyA: return ImGuiKey_A;
	case EKeyCode::eKeyB: return ImGuiKey_B;
	case EKeyCode::eKeyC: return ImGuiKey_C;
	case EKeyCode::eKeyD: return ImGuiKey_D;
	case EKeyCode::eKeyE: return ImGuiKey_E;
	case EKeyCode::eKeyF: return ImGuiKey_F;
	case EKeyCode::eKeyG: return ImGuiKey_G;
	case EKeyCode::eKeyH: return ImGuiKey_H;
	case EKeyCode::eKeyI: return ImGuiKey_I;
	case EKeyCode::eKeyJ: return ImGuiKey_J;
	case EKeyCode::eKeyK: return ImGuiKey_K;
	case EKeyCode::eKeyL: return ImGuiKey_L;
	case EKeyCode::eKeyM: return ImGuiKey_M;
	case EKeyCode::eKeyN: return ImGuiKey_N;
	case EKeyCode::eKeyO: return ImGuiKey_O;
	case EKeyCode::eKeyP: return ImGuiKey_P;
	case EKeyCode::eKeyQ: return ImGuiKey_Q;
	case EKeyCode::eKeyR: return ImGuiKey_R;
	case EKeyCode::eKeyS: return ImGuiKey_S;
	case EKeyCode::eKeyT: return ImGuiKey_T;
	case EKeyCode::eKeyU: return ImGuiKey_U;
	case EKeyCode::eKeyV: return ImGuiKey_V;
	case EKeyCode::eKeyW: return ImGuiKey_W;
	case EKeyCode::eKeyX: return ImGuiKey_X;
	case EKeyCode::eKeyY: return ImGuiKey_Y;
	case EKeyCode::eKeyZ: return ImGuiKey_Z;
	case EKeyCode::eKeyF1: return ImGuiKey_F1;
	case EKeyCode::eKeyF2: return ImGuiKey_F2;
	case EKeyCode::eKeyF3: return ImGuiKey_F3;
	case EKeyCode::eKeyF4: return ImGuiKey_F4;
	case EKeyCode::eKeyF5: return ImGuiKey_F5;
	case EKeyCode::eKeyF6: return ImGuiKey_F6;
	case EKeyCode::eKeyF7: return ImGuiKey_F7;
	case EKeyCode::eKeyF8: return ImGuiKey_F8;
	case EKeyCode::eKeyF9: return ImGuiKey_F9;
	case EKeyCode::eKeyF10: return ImGuiKey_F10;
	case EKeyCode::eKeyF11: return ImGuiKey_F11;
	case EKeyCode::eKeyF12: return ImGuiKey_F12;
		//case EKeyCode::eKeyF13: return ImGuiKey_F13;
		//case EKeyCode::eKeyF14: return ImGuiKey_F14;
		//case EKeyCode::eKeyF15: return ImGuiKey_F15;
		//case EKeyCode::eKeyF16: return ImGuiKey_F16;
		//case EKeyCode::eKeyF17: return ImGuiKey_F17;
		//case EKeyCode::eKeyF18: return ImGuiKey_F18;
		//case EKeyCode::eKeyF19: return ImGuiKey_F19;
		//case EKeyCode::eKeyF20: return ImGuiKey_F20;
		//case EKeyCode::eKeyF21: return ImGuiKey_F21;
		//case EKeyCode::eKeyF22: return ImGuiKey_F22;
		//case EKeyCode::eKeyF23: return ImGuiKey_F23;
		//case EKeyCode::eKeyF24: return ImGuiKey_F24;
	default: return ImGuiKey_None;
	}
}

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
	m_vertexHandler = m_pRenderInterface->CreateVertexHandler("imgui_vertices", vertex_handler_ci);

	// Create font texture (just load) May be needs to implement texture interface
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	u64 upload_size = width * height * 4 * sizeof(char);

	FTextureCreateInfo create_info{};
	create_info.format = EApiDataFormat::eRGBA8_UNORM;
	create_info.flags = ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eTransferable | ETextureCreateFlagBits::e2D;
	create_info.SetFixedSize2D(width, height);

	FTextureUploadInfo upload_info{};
	upload_info.data.insert(upload_info.data.end(), pixels, pixels + upload_size);

	auto& render_context = m_pRenderInterface->GetRenderContext();
	m_fontTexture = m_pRenderInterface->CreateTextureObject("imgui_font", create_info, std::move(upload_info));

	// Make RHI shader object
	auto& vertex_handler = m_pRenderInterface->GetVertexHandler(m_vertexHandler);
	m_shader = MakeShader(pass_ci.shader, EShaderCullModeFlagBits::eNone, EShaderFrontFace::eCCW, true, EDepthCompareOp::eLessOrEqual, vertex_handler.get());
	InitShaderResources(m_shader);

	// Window events
	FWindowCallbackMapper::SubscribeFocusChange(this, &CImGuiPass::HandleFocusChanged);
	FWindowCallbackMapper::SubscribeCursorEnter(this, &CImGuiPass::HandleCursorEnter);
	FWindowCallbackMapper::SubscribeMousePosition(this, &CImGuiPass::HandleAxisInput);
	FWindowCallbackMapper::SubscribeMouseButton(this, &CImGuiPass::HandleMouseButton);
	FWindowCallbackMapper::SubscribeMouseScroll(this, &CImGuiPass::HandleMouseScroll);
	FWindowCallbackMapper::SubscribeKeyInput(this, &CImGuiPass::HandleKeyInput);
	FWindowCallbackMapper::SubscribeCharInput(this, &CImGuiPass::HandleCharInput);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = std::format("imgui:{}", pass_ci.shader);
#endif
}

void CImGuiPass::Build(const FPassBuildInfo& build_info)
{
	IRenderPass::Build(build_info);
}

void CImGuiPass::Update(f32 dt)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto resolution = render_context->GetScreenRenderRect();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(resolution.size.x, resolution.size.y);
	io.DeltaTime = dt;

	ImGui::NewFrame();

	static bool show_demo_window{ true };
	ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Render();

	ImDrawData* draw_data = ImGui::GetDrawData();
	auto& vertex_handler = m_pRenderInterface->GetVertexHandler(m_vertexHandler);

	m_lastVertex = 0;
	m_lastIndex = 0;

	// Update vertices
	if (draw_data->TotalVtxCount > 0)
	{
		// Upload vertex/index data into a single contiguous GPU buffer
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			vertex_handler->AddVertices(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size, sizeof(ImDrawVert));
			vertex_handler->AddIndices(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size, sizeof(ImDrawIdx));

			m_lastVertex += cmd_list->VtxBuffer.Size;
			m_lastIndex += cmd_list->IdxBuffer.Size;
		}
	}

	// Upload vertices
	vertex_handler->Allocate();
}

void CImGuiPass::_Render(const FRenderFrameInfo& frame_info)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto resolution = render_context->GetScreenRenderRect();

	ImGuiIO& io = ImGui::GetIO();

	// Render ingui
	{
		ImDrawData* draw_data = ImGui::GetDrawData();
		auto& vertex_handler = m_pRenderInterface->GetVertexHandler(m_vertexHandler);
		auto& shader = m_pRenderInterface->GetShaderObject(m_shader);

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

		FRHIHandle last_bound_resource{ INVALID_HANDLE };

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
					FRHIHandle attachment{ (u64)pcmd->TextureId };
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
						shader->BindTexture(utl::fnv1a_64_hash_cstr("fontSampler"), m_pRenderInterface->GetTextureObject(attachment)->Handle());
						shader->FlushDescriptors();
						shader->BindResources(m_queue);
						last_bound_resource = attachment;
					}

					FGeometryDrawInfo draw_info{};
					draw_info.lastVertex = m_lastVertex;
					draw_info.lastIndex = m_lastIndex;

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

void CImGuiPass::HandleFocusChanged(i32 focused)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddFocusEvent(focused);
}

void CImGuiPass::HandleCursorEnter(i32 enter)
{
	ImGuiIO& io = ImGui::GetIO();
	if (enter)
	{
		io.AddMousePosEvent(m_lastMouseX, m_lastMouseY);
	}
	else
	{
		m_lastMouseX = io.MousePos.x;
		m_lastMouseY = io.MousePos.y;
		io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
	}
}

void CImGuiPass::HandleAxisInput(f32 x, f32 y, f32 xmax, f32 ymax)
{
	ImGuiIO& io = ImGui::GetIO();

	// TODO: do update only when window is focused

	io.AddMousePosEvent(x, y);
	m_lastMouseX = x;
	m_lastMouseY = y;
}

void CImGuiPass::HandleMouseButton(EKeyCode code, EKeyState state)
{
	int button{ -1 };
	if (code == EKeyCode::eMouseLeft)
		button = 0;
	else if (code == EKeyCode::eMouseRight)
		button = 1;
	else if (code == EKeyCode::eMouseMiddle)
		button = 2;

	ImGuiIO& io = ImGui::GetIO();
	if (button >= 0 && button < ImGuiMouseButton_COUNT)
		io.AddMouseButtonEvent(button, state == EKeyState::ePress);
}

void CImGuiPass::HandleMouseScroll(f32 x, f32 y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseWheelEvent(x, y);
}

void CImGuiPass::HandleKeyInput(EKeyCode code, EKeyState state)
{
	if (state != EKeyState::ePress && state != EKeyState::eRelease)
		return;

	ImGuiIO& io = ImGui::GetIO();
	ImGuiKey imgui_key = key_code_to_im_gui_key(code);
	io.AddKeyEvent(imgui_key, (state == EKeyState::ePress));
	//io.SetKeyEventNativeData(imgui_key, keycode, scancode);
}

void CImGuiPass::HandleCharInput(u32 character)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(character);
}