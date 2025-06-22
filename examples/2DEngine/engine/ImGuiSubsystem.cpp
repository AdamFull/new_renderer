#include "ImGuiSubsystem.h"
#include "render/IRenderer.h"

#include <imgui.h>

#include <edge/fs/fs_virtual_filesystem.h>
#include <edge/gfx/gfx_context.h>
#include <edge/gfx/gfx_resource_loader.h>

#include <edge/util/Logger.h>

#include "../assets/graphics/shaders/imgui.h"

namespace e2d {
	constexpr uint64_t initial_vertex_count = 16384ull;
	constexpr uint64_t initial_index_count = initial_vertex_count * 3ull;

	ImGuiKey key_code_to_imgui_key(edge::KeyCode code) {
		switch (code) {
		case edge::KeyCode::Tab: return ImGuiKey_Tab;
		case edge::KeyCode::Left: return ImGuiKey_LeftArrow;
		case edge::KeyCode::Right: return ImGuiKey_RightArrow;
		case edge::KeyCode::Up: return ImGuiKey_UpArrow;
		case edge::KeyCode::Down: return ImGuiKey_DownArrow;
		case edge::KeyCode::PageUp: return ImGuiKey_PageUp;
		case edge::KeyCode::PageDown: return ImGuiKey_PageDown;
		case edge::KeyCode::Home: return ImGuiKey_Home;
		case edge::KeyCode::End: return ImGuiKey_End;
		case edge::KeyCode::Insert: return ImGuiKey_Insert;
		case edge::KeyCode::DelKey: return ImGuiKey_Delete;
		case edge::KeyCode::Backspace: return ImGuiKey_Backspace;
		case edge::KeyCode::Space: return ImGuiKey_Space;
		case edge::KeyCode::Enter: return ImGuiKey_Enter;
		case edge::KeyCode::Escape: return ImGuiKey_Escape;
		case edge::KeyCode::Apostrophe: return ImGuiKey_Apostrophe;
		case edge::KeyCode::Comma: return ImGuiKey_Comma;
		case edge::KeyCode::Minus: return ImGuiKey_Minus;
		case edge::KeyCode::Period: return ImGuiKey_Period;
		case edge::KeyCode::Slash: return ImGuiKey_Slash;
		case edge::KeyCode::Semicolon: return ImGuiKey_Semicolon;
		case edge::KeyCode::Equal: return ImGuiKey_Equal;
		case edge::KeyCode::LeftBracket: return ImGuiKey_LeftBracket;
		case edge::KeyCode::Backslash: return ImGuiKey_Backslash;
		case edge::KeyCode::RightBracket: return ImGuiKey_RightBracket;
		case edge::KeyCode::GraveAccent: return ImGuiKey_GraveAccent;
		case edge::KeyCode::CapsLock: return ImGuiKey_CapsLock;
		case edge::KeyCode::ScrollLock: return ImGuiKey_ScrollLock;
		case edge::KeyCode::NumLock: return ImGuiKey_NumLock;
		case edge::KeyCode::PrintScreen: return ImGuiKey_PrintScreen;
		case edge::KeyCode::Pause: return ImGuiKey_Pause;
		case edge::KeyCode::KP_0: return ImGuiKey_Keypad0;
		case edge::KeyCode::KP_1: return ImGuiKey_Keypad1;
		case edge::KeyCode::KP_2: return ImGuiKey_Keypad2;
		case edge::KeyCode::KP_3: return ImGuiKey_Keypad3;
		case edge::KeyCode::KP_4: return ImGuiKey_Keypad4;
		case edge::KeyCode::KP_5: return ImGuiKey_Keypad5;
		case edge::KeyCode::KP_6: return ImGuiKey_Keypad6;
		case edge::KeyCode::KP_7: return ImGuiKey_Keypad7;
		case edge::KeyCode::KP_8: return ImGuiKey_Keypad8;
		case edge::KeyCode::KP_9: return ImGuiKey_Keypad9;
		case edge::KeyCode::KP_Decimal: return ImGuiKey_KeypadDecimal;
		case edge::KeyCode::KP_Divide: return ImGuiKey_KeypadDivide;
		case edge::KeyCode::KP_Multiply: return ImGuiKey_KeypadMultiply;
		case edge::KeyCode::KP_Subtract: return ImGuiKey_KeypadSubtract;
		case edge::KeyCode::KP_Add: return ImGuiKey_KeypadAdd;
		case edge::KeyCode::KP_Enter: return ImGuiKey_KeypadEnter;
		case edge::KeyCode::KP_Equal: return ImGuiKey_KeypadEqual;
		case edge::KeyCode::LeftShift: return ImGuiKey_LeftShift;
		case edge::KeyCode::LeftControl: return ImGuiKey_LeftCtrl;
		case edge::KeyCode::LeftAlt: return ImGuiKey_LeftAlt;
			//case GLFW_KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
		case edge::KeyCode::RightShift: return ImGuiKey_RightShift;
		case edge::KeyCode::RightControl: return ImGuiKey_RightCtrl;
		case edge::KeyCode::RightAlt: return ImGuiKey_RightAlt;
			//case GLFW_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
			//case GLFW_KEY_MENU: return ImGuiKey_Menu;
		case edge::KeyCode::_0: return ImGuiKey_0;
		case edge::KeyCode::_1: return ImGuiKey_1;
		case edge::KeyCode::_2: return ImGuiKey_2;
		case edge::KeyCode::_3: return ImGuiKey_3;
		case edge::KeyCode::_4: return ImGuiKey_4;
		case edge::KeyCode::_5: return ImGuiKey_5;
		case edge::KeyCode::_6: return ImGuiKey_6;
		case edge::KeyCode::_7: return ImGuiKey_7;
		case edge::KeyCode::_8: return ImGuiKey_8;
		case edge::KeyCode::_9: return ImGuiKey_9;
		case edge::KeyCode::A: return ImGuiKey_A;
		case edge::KeyCode::B: return ImGuiKey_B;
		case edge::KeyCode::C: return ImGuiKey_C;
		case edge::KeyCode::D: return ImGuiKey_D;
		case edge::KeyCode::E: return ImGuiKey_E;
		case edge::KeyCode::F: return ImGuiKey_F;
		case edge::KeyCode::G: return ImGuiKey_G;
		case edge::KeyCode::H: return ImGuiKey_H;
		case edge::KeyCode::I: return ImGuiKey_I;
		case edge::KeyCode::J: return ImGuiKey_J;
		case edge::KeyCode::K: return ImGuiKey_K;
		case edge::KeyCode::L: return ImGuiKey_L;
		case edge::KeyCode::M: return ImGuiKey_M;
		case edge::KeyCode::N: return ImGuiKey_N;
		case edge::KeyCode::O: return ImGuiKey_O;
		case edge::KeyCode::P: return ImGuiKey_P;
		case edge::KeyCode::Q: return ImGuiKey_Q;
		case edge::KeyCode::R: return ImGuiKey_R;
		case edge::KeyCode::S: return ImGuiKey_S;
		case edge::KeyCode::T: return ImGuiKey_T;
		case edge::KeyCode::U: return ImGuiKey_U;
		case edge::KeyCode::V: return ImGuiKey_V;
		case edge::KeyCode::W: return ImGuiKey_W;
		case edge::KeyCode::X: return ImGuiKey_X;
		case edge::KeyCode::Y: return ImGuiKey_Y;
		case edge::KeyCode::Z: return ImGuiKey_Z;
		case edge::KeyCode::F1: return ImGuiKey_F1;
		case edge::KeyCode::F2: return ImGuiKey_F2;
		case edge::KeyCode::F3: return ImGuiKey_F3;
		case edge::KeyCode::F4: return ImGuiKey_F4;
		case edge::KeyCode::F5: return ImGuiKey_F5;
		case edge::KeyCode::F6: return ImGuiKey_F6;
		case edge::KeyCode::F7: return ImGuiKey_F7;
		case edge::KeyCode::F8: return ImGuiKey_F8;
		case edge::KeyCode::F9: return ImGuiKey_F9;
		case edge::KeyCode::F10: return ImGuiKey_F10;
		case edge::KeyCode::F11: return ImGuiKey_F11;
		case edge::KeyCode::F12: return ImGuiKey_F12;
			//case edge::KeyCode::eKeyF13: return ImGuiKey_F13;
			//case edge::KeyCode::eKeyF14: return ImGuiKey_F14;
			//case edge::KeyCode::eKeyF15: return ImGuiKey_F15;
			//case edge::KeyCode::eKeyF16: return ImGuiKey_F16;
			//case edge::KeyCode::eKeyF17: return ImGuiKey_F17;
			//case edge::KeyCode::eKeyF18: return ImGuiKey_F18;
			//case edge::KeyCode::eKeyF19: return ImGuiKey_F19;
			//case edge::KeyCode::eKeyF20: return ImGuiKey_F20;
			//case edge::KeyCode::eKeyF21: return ImGuiKey_F21;
			//case edge::KeyCode::eKeyF22: return ImGuiKey_F22;
			//case edge::KeyCode::eKeyF23: return ImGuiKey_F23;
			//case edge::KeyCode::eKeyF24: return ImGuiKey_F24;
		default: return ImGuiKey_None;
		}
	}

	ImGuiSubsystem::ImGuiSubsystem(IRenderer& renderer) :
		renderer_{ renderer } {
	}

	ImGuiSubsystem::~ImGuiSubsystem() {
		ImGui::EndFrame();
		ImGui::DestroyContext();
	}

	auto ImGuiSubsystem::construct(IRenderer& renderer, OnSubsystemProcessCb cb) -> std::unique_ptr<ISubsystem> {
		auto self = std::make_unique<ImGuiSubsystem>(renderer);
		self->initialize();
		self->set_on_process(cb);
		return self;
	}

	auto ImGuiSubsystem::initialize() -> void {
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		IMGUI_CHECKVERSION();
		IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

		io.BackendRendererUserData = (void*)this;
		io.BackendRendererName = "e2d";
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
		//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can Create multi-viewports on the Renderer side (optional)

		// Create font texture
		update_font_texture();

		// Create geometry buffers
		edge::gfx::BufferCreateInfo vertex_buffer_ci{};
		vertex_buffer_ci.size = sizeof(ImDrawVert);
		vertex_buffer_ci.count = initial_vertex_count;
		vertex_buffer_ci.usage = edge::gfx::BufferUsageFlag::eVertexBufferCPU;
		vertex_buffer_ = edge::gfx::create_buffer(vertex_buffer_ci);

		edge::gfx::BufferCreateInfo index_buffer_ci{};
		index_buffer_ci.size = sizeof(ImDrawIdx);
		index_buffer_ci.count = initial_index_count;
		index_buffer_ci.usage = edge::gfx::BufferUsageFlag::eIndexBufferCPU;
		index_buffer_ = edge::gfx::create_buffer(index_buffer_ci);

		edge::gfx::SamplerCreateInfo sampler_ci{};
		sampler_ci.min_filter = edge::gfx::Filter::eLinear;
		sampler_ci.mag_filter = edge::gfx::Filter::eLinear;
		sampler_ci.mip_map_mode = edge::gfx::Filter::eLinear;
		sampler_ci.address_u = edge::gfx::SamplerAddressMode::eClampToEdge;
		sampler_ci.address_v = edge::gfx::SamplerAddressMode::eClampToEdge;
		sampler_ci.address_w = edge::gfx::SamplerAddressMode::eClampToEdge;
		sampler_ = edge::gfx::create_sampler(sampler_ci);
	}

	auto ImGuiSubsystem::set_on_process(OnSubsystemProcessCb cb) -> void {
		on_process_ = cb;
	}

	auto ImGuiSubsystem::process(float delta_time) -> void {
		auto& render_graph = renderer_.get_render_graph();

		// Setup pass execution callbacks
		render_graph.add_pass({
			.name = "imgui",
			.technique_name = "imgui",
			.setup_func = [this](IRGPass& pass) { setup_resources(pass); },
			.execute_func = [this](IRGPass& pass, edge::gfx::ICommandList& cmd_list, float delta_time) { execute_graph(pass, cmd_list, delta_time); }
			});

		auto output_target = renderer_.get_backbuffer_resource();
		auto* backbuffer = render_graph.get_resource(output_target)->get_resource()->as<edge::gfx::ITexture*>();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(backbuffer->get_width()), static_cast<float>(backbuffer->get_height()));
		io.DeltaTime = delta_time;

		ImGui::NewFrame();

		if (on_process_) {
			on_process_(delta_time);
		}
	}

	auto ImGuiSubsystem::on_input_event(const edge::InputEvent& input_event) -> void {
		ImGuiIO& io = ImGui::GetIO();

		switch (input_event.get_source())
		{
		case edge::EventSource::Keyboard:
		{
			const auto& key_input = static_cast<const edge::KeyInputEvent&>(input_event);
			auto action = key_input.get_action();
			if (action != edge::KeyAction::Down && action != edge::KeyAction::Up) {
				return;
			}

			io.AddKeyEvent(key_code_to_imgui_key(key_input.get_code()), (action == edge::KeyAction::Down));
			//io.SetKeyEventNativeData(imgui_key, keycode, scancode);
		} break;
		case edge::EventSource::Character:
		{
			const auto& char_input = static_cast<const edge::CharacterInputEvent&>(input_event);
			io.AddInputCharacter(char_input.get_character_code());
		} break;
		case edge::EventSource::Mouse:
		{
			const auto& mouse_input = static_cast<const edge::MouseButtonInputEvent&>(input_event);
			auto action = mouse_input.get_action();
			switch (action)
			{
			case edge::MouseAction::Down:
			case edge::MouseAction::Up:
			{
				int button{ -1 };
				switch (mouse_input.get_button())
				{
				case edge::KeyCode::MouseLeft:
					button = 0;
					break;
				case edge::KeyCode::MouseRight:
					button = 1;
					break;
				case edge::KeyCode::MouseMiddle:
					button = 2;
					break;
				}

				if (button >= 0 && button < ImGuiMouseButton_COUNT)
					io.AddMouseButtonEvent(button, action == edge::MouseAction::Down);
			} break;
			case edge::MouseAction::Move:
			{
				// TODO: do update only when window is focused

				io.AddMousePosEvent(mouse_input.get_value_x(), mouse_input.get_value_y());
				last_mouse_x = mouse_input.get_value_x();
				last_mouse_y = mouse_input.get_value_y();
			} break;
			case edge::MouseAction::Scroll:
			{
				io.AddMouseWheelEvent(mouse_input.get_value_x(), mouse_input.get_value_y());
			} break;
			default:
				break;
			}
		} break;
		case edge::EventSource::Touchscreen:
		{
			const auto& touch_input = static_cast<const edge::TouchInputEvent&>(input_event);
			// TODO:
		} break;
		}
	}

	auto ImGuiSubsystem::load_font(std::string_view path) -> void {

	}

	auto ImGuiSubsystem::setup_resources(IRGPass& pass) -> void {
		ImGuiIO& io = ImGui::GetIO();

		auto output_target = renderer_.get_backbuffer_resource();
		pass.add_color_attachment(output_target, edge::gfx::LoadOp::eClear);
		pass.set_size(io.DisplaySize.x, io.DisplaySize.y);
	}

	auto ImGuiSubsystem::execute_graph(IRGPass& pass, edge::gfx::ICommandList& cmd_list, float delta_time) -> void {
		if (!ImGui::GetCurrentContext()) {
			return;
		}

		ImGui::Render();

		// Ubdate geometry
		ImDrawData* draw_data = ImGui::GetDrawData();
		if (draw_data && (draw_data->TotalVtxCount != 0) && (draw_data->TotalIdxCount != 0)) {
			std::vector<ImDrawVert> vertices{};
			std::vector<ImDrawIdx> indices{};

			for (int32_t i = 0; i < draw_data->CmdListsCount; i++) {
				const ImDrawList* cmd_list = draw_data->CmdLists[i];

				auto* vtx_end = cmd_list->VtxBuffer.Data + cmd_list->VtxBuffer.Size;
				vertices.insert(vertices.end(), cmd_list->VtxBuffer.Data, vtx_end);

				auto* idx_end = cmd_list->IdxBuffer.Data + cmd_list->IdxBuffer.Size;
				indices.insert(indices.end(), cmd_list->IdxBuffer.Data, idx_end);
			}

			// Ubdate vertices
			if (!vertices.empty()) {
				auto total_size = vertices.size() * sizeof(ImDrawVert);
				if (total_size > vertex_buffer_->get_size()) {
					edge::gfx::destroy(vertex_buffer_);

					edge::gfx::BufferCreateInfo create_info{};
					create_info.count = vertices.size();
					create_info.size = sizeof(ImDrawVert);
					create_info.usage = edge::gfx::BufferUsageFlag::eVertexBufferCPU;
					vertex_buffer_ = edge::gfx::create_buffer(create_info);
				}

				vertex_buffer_->update(vertices.data(), total_size);
			}

			// Update indices
			if (!indices.empty()) {
				auto total_size = indices.size() * sizeof(ImDrawIdx);
				if (total_size > index_buffer_->get_size()) {
					edge::gfx::destroy(index_buffer_);

					edge::gfx::BufferCreateInfo create_info{};
					create_info.count = vertices.size();
					create_info.size = sizeof(ImDrawIdx);
					create_info.usage = edge::gfx::BufferUsageFlag::eIndexBufferCPU;
					index_buffer_ = edge::gfx::create_buffer(create_info);
				}

				index_buffer_->update(indices.data(), total_size);
			}
		}

		cmd_list.bind_index_buffer(index_buffer_, sizeof(ImDrawIdx) == 2 ? edge::gfx::IndexType::eUint16 : edge::gfx::IndexType::eUint32, 0ull);

		auto& graph = pass.get_graph();

		ImGuiIO& io = ImGui::GetIO();
		cmd_list.set_viewport(0.f, 0.f, io.DisplaySize.x, io.DisplaySize.y, 0.f, 1.f);

		// Setup scale and translation:
		// Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
		{
			imgui::PushConstant pc{};
			pc.scale.x = 2.f / draw_data->DisplaySize.x;
			pc.scale.y = 2.f / draw_data->DisplaySize.y;
			pc.translate.x = -1.0f - draw_data->DisplayPos.x * pc.scale.x;
			pc.translate.y = -1.0f - draw_data->DisplayPos.y * pc.scale.y;
			pc.vertices = vertex_buffer_->get_address();
			pass.push_constants(cmd_list, { (const char*)&pc, sizeof(pc) });
		}

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Collect resource dependencies
		for (int32_t n = 0; n < draw_data->CmdListsCount; n++) {
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			for (int32_t cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

				edge::gfx::ITexture* resource_binding{ (edge::gfx::ITexture*)pcmd->TextureId };
				if (sizeof(ImTextureID) < sizeof(ImU64)) {
					// We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a flaky check that other textures haven't been used.
					IM_ASSERT(pcmd->TextureId == (ImTextureID)font_image_);
					resource_binding = font_image_;
				}

				if (!pcmd->TextureId) {
					resource_binding = font_image_;
				}

				//self.read_resource(render_label::texture0, resource_binding, ~0u);
				//self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);
			}
		}

		// TODO: bind resource

		edge::gfx::ITexture* last_bound_resource{ nullptr };

		// Render command lists
		// (Because we merged all buffers into a single one, we maintain our own offset into them)
		int32_t global_vtx_offset = 0;
		int32_t global_idx_offset = 0;
		for (int32_t n = 0; n < draw_data->CmdListsCount; n++) {
			const ImDrawList* im_cmd_list = draw_data->CmdLists[n];
			for (int32_t cmd_i = 0; cmd_i < im_cmd_list->CmdBuffer.Size; cmd_i++) {
				const ImDrawCmd* pcmd = &im_cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback != nullptr) {
					// User callback, registered via ImDrawList::AddCallback()
					// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
					//if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					//	ImGui_ImplVulkan_SetupRenderState(draw_data, pipeline, command_buffer, rb, fb_width, fb_height);
					//else
					pcmd->UserCallback(im_cmd_list, pcmd);
				}
				else {
					// Project scissor/clipping rectangles into framebuffer space
					ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
					ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

					// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
					if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
					if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
					if (clip_max.x > io.DisplaySize.x) { clip_max.x = io.DisplaySize.x; }
					if (clip_max.y > io.DisplaySize.y) { clip_max.y = io.DisplaySize.y; }
					if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
						continue;

					// Apply scissor/clipping rectangle
					cmd_list.set_scissor((int32_t)(clip_min.x), (int32_t)(clip_min.y), (uint32_t)(clip_max.x - clip_min.x), (uint32_t)(clip_max.y - clip_min.y));

					// Bind DescriptorSet with font or user texture
					edge::gfx::ITexture* resource_binding{ (edge::gfx::ITexture*)pcmd->TextureId };
					if (sizeof(ImTextureID) < sizeof(ImU64)) {
						// We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a flaky check that other textures haven't been used.
						IM_ASSERT(pcmd->TextureId == (ImTextureID)font_image_);
						resource_binding = font_image_;
					}

					if (!pcmd->TextureId) {
						resource_binding = font_image_;
					}

					if (resource_binding != last_bound_resource) {
						edge::gfx::TextureDescriptor texture_info{};
						texture_info.type = edge::gfx::DescriptorType::eSampledImage;
						texture_info.texture = resource_binding;
						texture_info.resource_state = edge::gfx::ResourceStateFlag::eShaderResource;

						edge::gfx::SamplerDescriptor sampler_info{};
						sampler_info.sampler = sampler_;
						sampler_info.binding = 1;

						auto* layout = pass.get_descriptor_layouts()[0];
						auto& binding_layout = renderer_.get_binding_layout(layout);
						binding_layout.bind(texture_info);
						binding_layout.bind(sampler_info);

						std::vector<edge::gfx::IBindingLayout*> layouts{};
						layouts.push_back(&binding_layout);

						pass.bind_descriptors(cmd_list, layouts);

						last_bound_resource = resource_binding;
					}

					cmd_list.draw_indexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, 1u, pcmd->VtxOffset + global_vtx_offset, 0u);
				}
			}
			global_idx_offset += im_cmd_list->IdxBuffer.Size;
			global_vtx_offset += im_cmd_list->VtxBuffer.Size;
		}
	}

	void ImGuiSubsystem::update_font_texture() {
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Build();

		unsigned char* pixels;
		int width, height;

		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		size_t upload_size = width * height * 4ull;

		edge::gfx::TextureCreateInfo create_info{};
		create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eTransferable;
		create_info.width = width;
		create_info.height = height;
		create_info.format = edge::gfx::Format::eR8G8B8A8Unorm;

		if (font_image_ && (font_image_->get_width() != width || font_image_->get_height() != height)) {
			edge::gfx::destroy(font_image_);
		}
		else if(font_image_) {
			return;
		}

		font_image_ = edge::gfx::create_texture(create_info);

		edge::gfx::ResourceStateFlags font_image_state{};
		edge::gfx::update_texture(font_image_, font_image_state, { pixels, upload_size });

		// TODO: check that texture is already created and that it needs to be recreated
	}
}