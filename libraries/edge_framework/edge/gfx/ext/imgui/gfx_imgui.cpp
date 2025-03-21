#include "gfx_imgui.h"

#include "../../gfx_context.h"
#include "../../../runtime/rt_input_events.h"
#include "../../../runtime/rt_window.h"
#include "../../../util/utl_profiler.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "ImguiVertex.h"
#include "ImguiFragment.h"

#include <glm/glm.hpp>

namespace edge
{
	namespace gfx
	{
		namespace ext
		{
			constexpr const uint32_t k_upload_buffer_size{ 8u * 1024 * 1024 };

			struct ConstantRange
			{
				glm::vec2 scale;
				glm::vec2 pos;
			};

			static ImGuiKey KeyCodeToImGuiKey(KeyCode code)
			{
				switch (code)
				{
				case KeyCode::Tab: return ImGuiKey_Tab;
				case KeyCode::Left: return ImGuiKey_LeftArrow;
				case KeyCode::Right: return ImGuiKey_RightArrow;
				case KeyCode::Up: return ImGuiKey_UpArrow;
				case KeyCode::Down: return ImGuiKey_DownArrow;
				case KeyCode::PageUp: return ImGuiKey_PageUp;
				case KeyCode::PageDown: return ImGuiKey_PageDown;
				case KeyCode::Home: return ImGuiKey_Home;
				case KeyCode::End: return ImGuiKey_End;
				case KeyCode::Insert: return ImGuiKey_Insert;
				case KeyCode::DelKey: return ImGuiKey_Delete;
				case KeyCode::Backspace: return ImGuiKey_Backspace;
				case KeyCode::Space: return ImGuiKey_Space;
				case KeyCode::Enter: return ImGuiKey_Enter;
				case KeyCode::Escape: return ImGuiKey_Escape;
				case KeyCode::Apostrophe: return ImGuiKey_Apostrophe;
				case KeyCode::Comma: return ImGuiKey_Comma;
				case KeyCode::Minus: return ImGuiKey_Minus;
				case KeyCode::Period: return ImGuiKey_Period;
				case KeyCode::Slash: return ImGuiKey_Slash;
				case KeyCode::Semicolon: return ImGuiKey_Semicolon;
				case KeyCode::Equal: return ImGuiKey_Equal;
				case KeyCode::LeftBracket: return ImGuiKey_LeftBracket;
				case KeyCode::Backslash: return ImGuiKey_Backslash;
				case KeyCode::RightBracket: return ImGuiKey_RightBracket;
				case KeyCode::GraveAccent: return ImGuiKey_GraveAccent;
				case KeyCode::CapsLock: return ImGuiKey_CapsLock;
				case KeyCode::ScrollLock: return ImGuiKey_ScrollLock;
				case KeyCode::NumLock: return ImGuiKey_NumLock;
				case KeyCode::PrintScreen: return ImGuiKey_PrintScreen;
				case KeyCode::Pause: return ImGuiKey_Pause;
				case KeyCode::KP_0: return ImGuiKey_Keypad0;
				case KeyCode::KP_1: return ImGuiKey_Keypad1;
				case KeyCode::KP_2: return ImGuiKey_Keypad2;
				case KeyCode::KP_3: return ImGuiKey_Keypad3;
				case KeyCode::KP_4: return ImGuiKey_Keypad4;
				case KeyCode::KP_5: return ImGuiKey_Keypad5;
				case KeyCode::KP_6: return ImGuiKey_Keypad6;
				case KeyCode::KP_7: return ImGuiKey_Keypad7;
				case KeyCode::KP_8: return ImGuiKey_Keypad8;
				case KeyCode::KP_9: return ImGuiKey_Keypad9;
				case KeyCode::KP_Decimal: return ImGuiKey_KeypadDecimal;
				case KeyCode::KP_Divide: return ImGuiKey_KeypadDivide;
				case KeyCode::KP_Multiply: return ImGuiKey_KeypadMultiply;
				case KeyCode::KP_Subtract: return ImGuiKey_KeypadSubtract;
				case KeyCode::KP_Add: return ImGuiKey_KeypadAdd;
				case KeyCode::KP_Enter: return ImGuiKey_KeypadEnter;
				case KeyCode::KP_Equal: return ImGuiKey_KeypadEqual;
				case KeyCode::LeftShift: return ImGuiKey_LeftShift;
				case KeyCode::LeftControl: return ImGuiKey_LeftCtrl;
				case KeyCode::LeftAlt: return ImGuiKey_LeftAlt;
					//case GLFW_KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
				case KeyCode::RightShift: return ImGuiKey_RightShift;
				case KeyCode::RightControl: return ImGuiKey_RightCtrl;
				case KeyCode::RightAlt: return ImGuiKey_RightAlt;
					//case GLFW_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
					//case GLFW_KEY_MENU: return ImGuiKey_Menu;
				case KeyCode::_0: return ImGuiKey_0;
				case KeyCode::_1: return ImGuiKey_1;
				case KeyCode::_2: return ImGuiKey_2;
				case KeyCode::_3: return ImGuiKey_3;
				case KeyCode::_4: return ImGuiKey_4;
				case KeyCode::_5: return ImGuiKey_5;
				case KeyCode::_6: return ImGuiKey_6;
				case KeyCode::_7: return ImGuiKey_7;
				case KeyCode::_8: return ImGuiKey_8;
				case KeyCode::_9: return ImGuiKey_9;
				case KeyCode::A: return ImGuiKey_A;
				case KeyCode::B: return ImGuiKey_B;
				case KeyCode::C: return ImGuiKey_C;
				case KeyCode::D: return ImGuiKey_D;
				case KeyCode::E: return ImGuiKey_E;
				case KeyCode::F: return ImGuiKey_F;
				case KeyCode::G: return ImGuiKey_G;
				case KeyCode::H: return ImGuiKey_H;
				case KeyCode::I: return ImGuiKey_I;
				case KeyCode::J: return ImGuiKey_J;
				case KeyCode::K: return ImGuiKey_K;
				case KeyCode::L: return ImGuiKey_L;
				case KeyCode::M: return ImGuiKey_M;
				case KeyCode::N: return ImGuiKey_N;
				case KeyCode::O: return ImGuiKey_O;
				case KeyCode::P: return ImGuiKey_P;
				case KeyCode::Q: return ImGuiKey_Q;
				case KeyCode::R: return ImGuiKey_R;
				case KeyCode::S: return ImGuiKey_S;
				case KeyCode::T: return ImGuiKey_T;
				case KeyCode::U: return ImGuiKey_U;
				case KeyCode::V: return ImGuiKey_V;
				case KeyCode::W: return ImGuiKey_W;
				case KeyCode::X: return ImGuiKey_X;
				case KeyCode::Y: return ImGuiKey_Y;
				case KeyCode::Z: return ImGuiKey_Z;
				case KeyCode::F1: return ImGuiKey_F1;
				case KeyCode::F2: return ImGuiKey_F2;
				case KeyCode::F3: return ImGuiKey_F3;
				case KeyCode::F4: return ImGuiKey_F4;
				case KeyCode::F5: return ImGuiKey_F5;
				case KeyCode::F6: return ImGuiKey_F6;
				case KeyCode::F7: return ImGuiKey_F7;
				case KeyCode::F8: return ImGuiKey_F8;
				case KeyCode::F9: return ImGuiKey_F9;
				case KeyCode::F10: return ImGuiKey_F10;
				case KeyCode::F11: return ImGuiKey_F11;
				case KeyCode::F12: return ImGuiKey_F12;
					//case KeyCode::eKeyF13: return ImGuiKey_F13;
					//case KeyCode::eKeyF14: return ImGuiKey_F14;
					//case KeyCode::eKeyF15: return ImGuiKey_F15;
					//case KeyCode::eKeyF16: return ImGuiKey_F16;
					//case KeyCode::eKeyF17: return ImGuiKey_F17;
					//case KeyCode::eKeyF18: return ImGuiKey_F18;
					//case KeyCode::eKeyF19: return ImGuiKey_F19;
					//case KeyCode::eKeyF20: return ImGuiKey_F20;
					//case KeyCode::eKeyF21: return ImGuiKey_F21;
					//case KeyCode::eKeyF22: return ImGuiKey_F22;
					//case KeyCode::eKeyF23: return ImGuiKey_F23;
					//case KeyCode::eKeyF24: return ImGuiKey_F24;
				default: return ImGuiKey_None;
				}
			}

			struct ImGuiUploadBuffer
			{
				IBuffer* buffer{ nullptr };
				uint64_t offset{ 0ull };
			};

			struct ImGuiContext
			{
				IContext* context{ nullptr };
				Window* window{ nullptr };

				IShader* shader{ nullptr };
				IRootSignature* root_signature{ nullptr };
				IPipeline* pipeline{ nullptr };

				std::vector<ImGuiUploadBuffer> upload_buffers{};

				RenderResourceDesc font_texture{};
				RenderResourceDesc vertex_buffer{};
				RenderResourceDesc index_buffer{};
			};

			static ImGuiContext* gpImguiContext{ nullptr };

			static void InitUploader()
			{
				assert(gpImguiContext && "ImGui Context is not initialized.");

				IContext& context = *gpImguiContext->context;

				BufferCreateInfo upload_buffer_ci{};
				upload_buffer_ci.size = k_upload_buffer_size;
				upload_buffer_ci.usage = BufferUsageFlag::eStagingBuffer;

				gpImguiContext->upload_buffers.push_back({
					.buffer = context.create_buffer(upload_buffer_ci),
					.offset = 0
					});
			}

			static auto LookUpUploadBuffer(uint64_t requested_size) -> ImGuiUploadBuffer&
			{
				assert(gpImguiContext && "ImGui Context is not initialized.");

				auto& upload_buffers = gpImguiContext->upload_buffers;

				auto found = std::find_if(upload_buffers.begin(), upload_buffers.end(), [requested_size](const ImGuiUploadBuffer& block)
					{
						return block.buffer->get_size() - block.offset > requested_size;
					});

				if (found != upload_buffers.end())
				{
					return *found;
				}

				BufferCreateInfo upload_buffer_ci{};
				upload_buffer_ci.size = k_upload_buffer_size;
				upload_buffer_ci.usage = BufferUsageFlag::eStagingBuffer;

				IContext& context = *gpImguiContext->context;
				return gpImguiContext->upload_buffers.emplace_back(ImGuiUploadBuffer{
					context.create_buffer(upload_buffer_ci), 0 });
			}

			static auto RequestUploadBuffer(uint64_t requested_size) -> std::tuple<IBuffer*, uint64_t>
			{
				auto& upload_buffer = LookUpUploadBuffer(requested_size);
				upload_buffer.offset += requested_size;
				return { upload_buffer.buffer, upload_buffer.offset - requested_size };
			}

			static void UploaderNextFrame()
			{
				assert(gpImguiContext && "ImGui Context is not initialized.");

				for (auto& uploader : gpImguiContext->upload_buffers)
				{
					uploader.offset = 0;
				}
			}

			static void UpdateBuffer(const ICommandList& command_list, RenderResourceDesc& resource_desc, std::span<const uint8_t> data)
			{
				auto* dst_buffer = resource_desc.resource->as<IBuffer*>();

				BufferBarrierDesc buffer_barrier{};
				buffer_barrier.buffer = dst_buffer;
				buffer_barrier.src_state = resource_desc.state;
				resource_desc.state = buffer_barrier.dst_state = ResourceStateFlag::eCopyDst;
				buffer_barrier.range.size = dst_buffer->get_size();

				ResourceBarrierInfo resource_barrier{};
				resource_barrier.buffer_barriers = { &buffer_barrier, 1 };
				command_list.resource_barrier(resource_barrier);

				auto [src_buffer, src_offset] = RequestUploadBuffer(data.size());
				src_buffer->update(data.data(), data.size(), src_offset);

				BufferCopyRegion regions[] = {
					{
						.src_offset = src_offset,
						.dst_offset = 0,
						.size = data.size()
					}
				};

				CopyBufferInfo copy_buffer_info{};
				copy_buffer_info.src_buffer = src_buffer;
				copy_buffer_info.dst_buffer = dst_buffer;
				copy_buffer_info.regions = regions;

				command_list.copy_buffer(copy_buffer_info);
			}

			static void UpdateTexture(ICommandList& command_list, RenderResourceDesc& resource_desc, int32_t width, int32_t height, std::span<const uint8_t> data)
			{
				auto* dst_texture = resource_desc.resource->as<ITexture*>();

				TextureBarrierDesc texture_barrier{};
				texture_barrier.texture = dst_texture;
				texture_barrier.src_state = resource_desc.state;
				resource_desc.state = texture_barrier.dst_state = ResourceStateFlag::eCopyDst;

				ResourceBarrierInfo resource_barrier{};
				resource_barrier.texture_barriers = { &texture_barrier, 1 };
				command_list.resource_barrier(resource_barrier);

				auto [src_buffer, src_offset] = RequestUploadBuffer(data.size());
				src_buffer->update(data.data(), data.size(), src_offset);

				CopyTextureInfo copy_texture_info{};
				copy_texture_info.src_buffer = src_buffer;
				copy_texture_info.dst_texture = dst_texture;
				copy_texture_info.copy_regions[0].buffer_offset = src_offset;
				copy_texture_info.copy_regions[0].size_x = width;
				copy_texture_info.copy_regions[0].size_y = height;
				//copy_texture_info.copy_region_count = 1;

				command_list.copy_texture(copy_texture_info);
			}

			void ImGui_CreateContext(const ImGuiContextCreateInfo& create_info)
			{
				IContext& context = *create_info.gfx_context;

				gpImguiContext = new ImGuiContext;
				gpImguiContext->context = create_info.gfx_context;
				gpImguiContext->window = create_info.window;

				InitUploader();

				// Create shader set
				const ShaderStageInfo stage_infos[] = {
					{
						.stage = ShaderStageFlag::eVertex,
						.code = {reinterpret_cast<const uint8_t*>(ImguiVertex), ImguiVertex_size}
					},
					{
						.stage = ShaderStageFlag::eFragment,
						.code = {reinterpret_cast<const uint8_t*>(ImguiFragment), ImguiFragment_size}
					}
				};

				gpImguiContext->shader = context.create_shader(stage_infos);

				// Create root signature
				RootSignatureCreateInfo root_sign_ci{};
				root_sign_ci.shader = gpImguiContext->shader;
				gpImguiContext->root_signature = context.create_root_signature(root_sign_ci);

				// Create rendering pipeline
				GraphicsPipelineCreateInfo pipeline_ci{};
				pipeline_ci.shader = gpImguiContext->shader;
				pipeline_ci.root_signature = gpImguiContext->root_signature;
				pipeline_ci.pipeline_state.primitive_topology = PrimitiveTopology::eTriangleList;
				// TODO: rewrite without using vertex layout
				pipeline_ci.pipeline_state.vertex_layout.attributes[0].binding = 0;
				pipeline_ci.pipeline_state.vertex_layout.attributes[0].location = 0;
				pipeline_ci.pipeline_state.vertex_layout.attributes[0].offset = offsetof(ImDrawVert, pos);
				pipeline_ci.pipeline_state.vertex_layout.attributes[0].format = Format::eR32G32Sfloat;
				pipeline_ci.pipeline_state.vertex_layout.attributes[1].binding = 0;
				pipeline_ci.pipeline_state.vertex_layout.attributes[1].location = 1;
				pipeline_ci.pipeline_state.vertex_layout.attributes[1].offset = offsetof(ImDrawVert, uv);
				pipeline_ci.pipeline_state.vertex_layout.attributes[1].format = Format::eR32G32Sfloat;
				pipeline_ci.pipeline_state.vertex_layout.attributes[2].binding = 0;
				pipeline_ci.pipeline_state.vertex_layout.attributes[2].location = 2;
				pipeline_ci.pipeline_state.vertex_layout.attributes[2].offset = offsetof(ImDrawVert, col);
				pipeline_ci.pipeline_state.vertex_layout.attributes[2].format = Format::eR8G8B8A8Unorm;
				pipeline_ci.pipeline_state.vertex_layout.attribute_count = 3;
				pipeline_ci.pipeline_state.vertex_layout.bindings[0].binding = 0;
				pipeline_ci.pipeline_state.vertex_layout.bindings[0].stride = sizeof(ImDrawVert);
				pipeline_ci.pipeline_state.vertex_layout.binding_count = 1;

				// Alpha blending setup
				pipeline_ci.pipeline_state.color_blending_state.attachments[0].enable = true;
				pipeline_ci.pipeline_state.color_blending_state.attachments[0].src_color = edge::gfx::BlendFactor::eSrcAlpha;
				pipeline_ci.pipeline_state.color_blending_state.attachments[0].dst_color = edge::gfx::BlendFactor::eOneMinusSrcAlpha;
				pipeline_ci.pipeline_state.color_blending_state.attachments[0].src_alpha = edge::gfx::BlendFactor::eOneMinusSrcAlpha;
				pipeline_ci.pipeline_state.color_blending_state.attachments[0].dst_alpha = edge::gfx::BlendFactor::eZero;
				pipeline_ci.pipeline_state.color_blending_state.attachment_count = 1;

				pipeline_ci.color_formats[0] = create_info.color_format;
				pipeline_ci.color_format_count = 1;

				gpImguiContext->pipeline = context.create_pipeline(pipeline_ci);

				constexpr const uint32_t k_startap_vertex_count{ 512u };

				BufferCreateInfo vertex_buffer_ci{};
				vertex_buffer_ci.size = sizeof(ImDrawVert) * k_startap_vertex_count;
				vertex_buffer_ci.usage = BufferUsageFlag::eVertexBufferCPU;
				gpImguiContext->vertex_buffer.resource = context.create_buffer(vertex_buffer_ci);

				BufferCreateInfo index_buffer_ci{};
				index_buffer_ci.size = sizeof(ImDrawIdx) * k_startap_vertex_count * 3;
				index_buffer_ci.usage = BufferUsageFlag::eIndexBufferCPU;
				gpImguiContext->index_buffer.resource = context.create_buffer(index_buffer_ci);

				ImGui::create_context();

				ImGuiIO& io = ImGui::GetIO();
				IMGUI_CHECKVERSION();
				IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

				io.BackendRendererUserData = (void*)gpImguiContext;
				io.BackendRendererName = "edge";
				io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
				//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can Create multi-viewports on the Renderer side (optional)
			}

			void ImGui_UpdateFontTexture(ICommandList& command_list)
			{
				EDGE_PROFILE_ZONE_SCOPED;
				assert(gpImguiContext && "ImGui Context is not initialized.");

				auto& context = *gpImguiContext->context;

				// Destroy texture and create new one
				if (gpImguiContext->font_texture.resource)
				{
					context.destroy(gpImguiContext->font_texture.resource);
					gpImguiContext->font_texture.resource = nullptr;
					gpImguiContext->font_texture.state = ResourceStateFlag::eUndefined;
				}

				ImGuiIO& io = ImGui::GetIO();
				io.Fonts->Build();

				uint8_t* font_pixels{ nullptr };
				int font_width, font_height;

				io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);
				uint32_t upload_size = font_width * font_height * 4u * sizeof(char);

				TextureCreateInfo font_texture_ci{};
				font_texture_ci.usage = TextureUsageFlag::eSamplable | TextureUsageFlag::eTransferable;
				font_texture_ci.width = font_width;
				font_texture_ci.height = font_height;
				font_texture_ci.format = Format::eR8G8B8A8Unorm;

				gpImguiContext->font_texture.resource = context.create_texture(font_texture_ci);

				UpdateTexture(command_list, gpImguiContext->font_texture, font_width, font_height, { font_pixels , upload_size });

				TextureBarrierDesc texture_barrier{};
				texture_barrier.texture = gpImguiContext->font_texture.resource->as<ITexture*>();
				texture_barrier.src_state = gpImguiContext->font_texture.state;
				gpImguiContext->font_texture.state = texture_barrier.dst_state = ResourceStateFlag::eShaderReadOnlyResource;

				ResourceBarrierInfo resource_barrier{};
				resource_barrier.texture_barriers = { &texture_barrier, 1 };

				// Transfer to final layout
				command_list.resource_barrier(resource_barrier);
			}

			static void HandleKeyboardEvent(const KeyInputEvent& input_event)
			{
				auto action = input_event.get_action();
				if (action != edge::KeyAction::Down && action != edge::KeyAction::Up)
					return;

				ImGuiIO& io = ImGui::GetIO();
				ImGuiKey imgui_key = KeyCodeToImGuiKey(input_event.get_code());
				io.AddKeyEvent(imgui_key, (action == KeyAction::Down));
				//io.SetKeyEventNativeData(imgui_key, keycode, scancode);
			}

			static void HandleCharacterEvent(const edge::CharacterInputEvent& input_event)
			{
				ImGuiIO& io = ImGui::GetIO();
				io.AddInputCharacter(input_event.get_character_code());
			}

			static void HandleMouseEvent(const edge::MouseButtonInputEvent& input_event)
			{
				auto action = input_event.get_action();
				switch (action)
				{
				case edge::MouseAction::Down:
				case edge::MouseAction::Up:
				{
					int button{ -1 };
					switch (input_event.get_button())
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

					ImGuiIO& io = ImGui::GetIO();
					if (button >= 0 && button < ImGuiMouseButton_COUNT)
						io.AddMouseButtonEvent(button, action == edge::MouseAction::Down);
				} break;
				case edge::MouseAction::Move:
				{
					ImGuiIO& io = ImGui::GetIO();
					// TODO: do update only when window is focused

					io.AddMousePosEvent(input_event.get_value_x(), input_event.get_value_y());
				} break;
				case edge::MouseAction::Scroll:
				{
					ImGuiIO& io = ImGui::GetIO();
					io.AddMouseWheelEvent(input_event.get_value_x(), input_event.get_value_y());
				} break;
				default:
					break;
				}
			}

			void ImGui_ProcessEvent(const InputEvent& input_event)
			{
				assert(gpImguiContext && "ImGui Context is not initialized.");

				switch (input_event.get_source())
				{
				case edge::EventSource::Keyboard:
				{
					const auto& key_input = static_cast<const edge::KeyInputEvent&>(input_event);
					HandleKeyboardEvent(key_input);
					
				} break;
				case edge::EventSource::Character:
				{
					const auto& char_input = static_cast<const edge::CharacterInputEvent&>(input_event);
					HandleCharacterEvent(char_input);
				} break;
				case edge::EventSource::Mouse:
				{
					const auto& mouse_input = static_cast<const edge::MouseButtonInputEvent&>(input_event);
					HandleMouseEvent(mouse_input);
				} break;
				case edge::EventSource::Touchscreen:
				{
					const auto& touch_input = static_cast<const edge::TouchInputEvent&>(input_event);
					assert(false && "NOT IMPLEMENTED");
				} break;
				}
			}

			void ImGui_NewFrame(float delta_time)
			{
				assert(gpImguiContext && "ImGui Context is not initialized.");

				auto& extent = gpImguiContext->window->get_extent();

				ImGuiIO& io = ImGui::GetIO();
				io.DisplaySize = ImVec2(static_cast<float>(extent.width), static_cast<float>(extent.height));
				io.DeltaTime = delta_time;

				ImGui::NewFrame();
			}

			void ImGui_Render(ICommandList& command_list, RenderResourceDesc& render_target)
			{
				EDGE_PROFILE_ZONE_SCOPED;
				assert(gpImguiContext && "ImGui Context is not initialized.");

				ImGui::Render();

				ImDrawData* draw_data = ImGui::GetDrawData();
				if (!draw_data)
					return;
				if ((draw_data->TotalVtxCount == 0) && (draw_data->TotalIdxCount == 0))
					return;

				// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
				int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
				int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
				if (fb_width <= 0 || fb_height <= 0)
					return;

				auto& context = *gpImguiContext->context;
				
				// Upload and bind vertex buffers
				if (draw_data->TotalVtxCount > 0)
				{
					static auto try_grow_buffer = [](RenderResourceDesc& resource_desc, int32_t requested_size, BufferUsageFlags usage)
						{
							auto* buffer = resource_desc.resource->as<IBuffer*>();
							if (buffer->get_size() < requested_size)
							{
								gpImguiContext->context->destroy(resource_desc.resource);
								resource_desc.state = ResourceStateFlag::eUndefined;

								BufferCreateInfo buffer_ci{};
								buffer_ci.size = requested_size;
								buffer_ci.usage = usage;
								resource_desc.resource = gpImguiContext->context->create_buffer(buffer_ci);

								return resource_desc.resource->as<IBuffer*>();
							}

							return buffer;
						};

					
					uint32_t total_vertices{ 0u };
					int32_t upload_vertices_size = sizeof(ImDrawVert) * draw_data->TotalVtxCount;
					auto* vertex_buffer = try_grow_buffer(gpImguiContext->vertex_buffer, upload_vertices_size, BufferUsageFlag::eVertexBufferCPU);
					auto* vertex_map = vertex_buffer->map();

					uint32_t total_indices{ 0u };
					int32_t upload_indices_size = sizeof(ImDrawIdx) * draw_data->TotalIdxCount;
					std::vector<BufferCopyRegion> idx_regions{};
					auto* index_buffer = try_grow_buffer(gpImguiContext->index_buffer, upload_vertices_size, BufferUsageFlag::eIndexBufferCPU);
					auto* index_map = index_buffer->map();

					for (int32_t i = 0; i < draw_data->CmdListsCount; i++)
					{
						const ImDrawList* cmd_list = draw_data->CmdLists[i];
						memcpy(vertex_map + total_vertices * sizeof(ImDrawVert), cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
						total_vertices += cmd_list->VtxBuffer.Size;

						memcpy(index_map + total_indices * sizeof(ImDrawIdx), cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
						total_indices += cmd_list->IdxBuffer.Size;
					}

					vertex_buffer->unmap();
					index_buffer->unmap();

					const uint32_t stride{ 0u };
					uint64_t offset{ 0ull };
					command_list.bind_vertex_buffers({ &vertex_buffer, 1 }, { &stride, 1 }, { &offset, 1 });
					command_list.bind_index_buffer(index_buffer, sizeof(ImDrawIdx) == 2 ? IndexType::eUint16 : IndexType::eUint32, 0);
				}

				// Bind shader
				command_list.bind_pipeline(gpImguiContext->pipeline);

				// Setup viewport
				command_list.set_viewport(0.f, 0.f, fb_width, fb_height, 0.f, 1.f);

				// Push constants
				{
					ConstantRange constant_range{};
					constant_range.scale.x = 2.f / draw_data->DisplaySize.x;
					constant_range.scale.y = 2.f / draw_data->DisplaySize.y;
					constant_range.pos.x = -1.0f - draw_data->DisplayPos.x * constant_range.scale.x;
					constant_range.pos.y = -1.0f - draw_data->DisplayPos.y * constant_range.scale.y;

					const char* range_ptr = reinterpret_cast<const char*>(&constant_range);
					command_list.push_constants(gpImguiContext->shader, gpImguiContext->root_signature, { range_ptr, sizeof(ConstantRange) });
				}

				if (draw_data && (draw_data->TotalVtxCount != 0) && (draw_data->TotalIdxCount != 0))
				{
					uint32_t total_vertices{ 0u };
					uint32_t total_indices{ 0u };

					//for (int32_t i = 0; i < draw_data->CmdListsCount; i++)
					//{
					//	const ImDrawList* cmd_list = draw_data->CmdLists[i];
					//	geometry_buffer.AddVertexStreamData(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size, sizeof(ImDrawVert));
					//	geometry_buffer.AddIndexStreamData(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size, sizeof(ImDrawIdx));
					//
					//	total_vertices += cmd_list->VtxBuffer.Size;
					//	total_indices += cmd_list->IdxBuffer.Size;
					//}
				}

				UploaderNextFrame();
			}

			void ImGui_DestroyContext()
			{
				assert(gpImguiContext && "ImGui Context is not initialized.");

				if (gpImguiContext->font_texture.resource)
				{
					gpImguiContext->context->destroy(gpImguiContext->font_texture.resource);
					gpImguiContext->font_texture.resource = nullptr;
				}

				ImGui::end_frame();

				delete gpImguiContext;
				gpImguiContext = nullptr;

				ImGui::DestroyContext();
			}
		}
	}
}