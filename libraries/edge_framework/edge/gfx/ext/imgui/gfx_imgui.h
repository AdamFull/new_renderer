#pragma once

#include "../../gfx_structures.h"

namespace edge
{
	class InputEvent;
	class Window;

	namespace gfx
	{
		class IContext;

		namespace ext
		{
			struct ImGuiContextCreateInfo
			{
				IContext* gfx_context{ nullptr };
				Window* window{ nullptr };
				Format color_format{ Format::eUndefined };
			};

			void ImGui_CreateContext(const ImGuiContextCreateInfo& create_info);
			void ImGui_UpdateFontTexture(ICommandList& command_list);
			void ImGui_ProcessEvent(const InputEvent& input_event);
			void ImGui_NewFrame(float delta_time);
			void ImGui_Render(ICommandList& command_list, RenderResourceDesc& render_target);
			void ImGui_DestroyContext();
		}
	}
}