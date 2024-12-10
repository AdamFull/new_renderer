#include "EngineUI.h"

#include <cstdint>

#include <imgui.h>
#include <imgui_internal.h>

#include "ImFontManager.h"
#include "font/IconsMaterialDesignIcons.h"

#include "../Window.h"

namespace engine
{
	void ApplyModernStyle()
	{
		// Modern style by LousyBook-01 from ImThemes
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.300000011920929f;
		style.WindowPadding = ImVec2(10.10000038146973f, 10.10000038146973f);
		style.WindowRounding = 10.30000019073486f;
		style.WindowBorderSize = 1.0f;
		style.WindowMinSize = ImVec2(20.0f, 32.0f);
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.ChildRounding = 8.199999809265137f;
		style.ChildBorderSize = 1.0f;
		style.PopupRounding = 10.69999980926514f;
		style.PopupBorderSize = 1.0f;
		style.FramePadding = ImVec2(20.0f, 1.5f);
		style.FrameRounding = 4.800000190734863f;
		style.FrameBorderSize = 0.0f;
		style.ItemSpacing = ImVec2(9.699999809265137f, 5.300000190734863f);
		style.ItemInnerSpacing = ImVec2(5.400000095367432f, 9.300000190734863f);
		style.CellPadding = ImVec2(7.900000095367432f, 2.0f);
		style.IndentSpacing = 10.69999980926514f;
		style.ColumnsMinSpacing = 6.0f;
		style.ScrollbarSize = 12.10000038146973f;
		style.ScrollbarRounding = 20.0f;
		style.GrabMinSize = 10.0f;
		style.GrabRounding = 4.599999904632568f;
		style.TabRounding = 4.0f;
		style.TabBorderSize = 0.0f;
		style.TabMinWidthForCloseButton = 0.0f;
		style.ColorButtonPosition = ImGuiDir_Right;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(1.0f, 1.0f, 1.0f, 0.3991416096687317f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f, 0.9399999976158142f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05098039284348488f, 0.05098039284348488f, 0.05098039284348488f, 0.9399999976158142f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.4206008315086365f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1411764770746231f, 0.1411764770746231f, 0.1411764770746231f, 0.4000000059604645f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2313725501298904f, 0.2313725501298904f, 0.2313725501298904f, 0.8626609444618225f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.09411764889955521f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.2918455004692078f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.9803921580314636f, 0.2588235437870026f, 0.2588235437870026f, 1.0f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9803921580314636f, 1.0f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.5793991088867188f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.2313725501298904f, 0.2313725501298904f, 1.0f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.454935610294342f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1803921610116959f, 0.1803921610116959f, 0.1803921610116959f, 0.800000011920929f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.9764705896377563f, 0.2588235437870026f, 0.2588235437870026f, 1.0f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 0.7799999713897705f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.2000000029802322f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.6700000166893005f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.949999988079071f);
		style.Colors[ImGuiCol_Tab] = ImVec4(0.105882354080677f, 0.105882354080677f, 0.105882354080677f, 1.0f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(1.0f, 0.364705890417099f, 0.6745098233222961f, 0.800000011920929f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(1.0f, 0.2235294133424759f, 0.2235294133424759f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1098039224743843f, 0.168627455830574f, 0.239215686917305f, 0.9724000096321106f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2156862765550613f, 0.2156862765550613f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.2156862765550613f, 0.6980392336845398f, 1.0f);
		style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(1.0f, 0.2352941185235977f, 0.2352941185235977f, 1.0f);
		style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(1.0f, 0.3176470696926117f, 0.3176470696926117f, 1.0f);
		style.Colors[ImGuiCol_TableBorderLight] = ImVec4(1.0f, 0.5647059082984924f, 0.5647059082984924f, 0.3690987229347229f);
		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.7254902124404907f, 0.3372549116611481f, 1.0f, 0.0f);
		style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 0.2745098173618317f, 0.2745098173618317f, 0.1115880012512207f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.9764705896377563f, 0.2588235437870026f, 0.2588235437870026f, 1.0f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 0.6437768340110779f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.4678111672401428f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.733905553817749f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.7982832789421082f);
	}

	void ApplyCinderTheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.300000011920929f;
		style.WindowPadding = ImVec2(10.10000038146973f, 10.10000038146973f);
		style.WindowRounding = 10.30000019073486f;
		style.WindowBorderSize = 1.0f;
		style.WindowMinSize = ImVec2(20.0f, 32.0f);
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.ChildRounding = 8.199999809265137f;
		style.ChildBorderSize = 1.0f;
		style.PopupRounding = 10.69999980926514f;
		style.PopupBorderSize = 1.0f;
		//style.FramePadding = ImVec2(20.0f, 1.5f);
		style.FrameRounding = 4.800000190734863f;
		style.FrameBorderSize = 0.0f;
		style.ItemSpacing = ImVec2(9.699999809265137f, 5.300000190734863f);
		style.ItemInnerSpacing = ImVec2(5.400000095367432f, 9.300000190734863f);
		style.CellPadding = ImVec2(7.900000095367432f, 2.0f);
		style.IndentSpacing = 10.69999980926514f;
		style.ColumnsMinSpacing = 6.0f;
		style.ScrollbarSize = 12.10000038146973f;
		style.ScrollbarRounding = 20.0f;
		style.GrabMinSize = 10.0f;
		style.GrabRounding = 4.599999904632568f;
		style.TabRounding = 4.0f;
		style.TabBorderSize = 0.0f;
		style.TabMinWidthForCloseButton = 0.0f;
		style.ColorButtonPosition = ImGuiDir_Right;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

		colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 0.50f);
		colors[ImGuiCol_Border] = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.47f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.92f, 0.18f, 0.29f, 0.76f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.92f, 0.18f, 0.29f, 0.43f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.9f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);

		colors[ImGuiCol_Separator] = colors[ImGuiCol_MenuBarBg];
		colors[ImGuiCol_SeparatorActive] = colors[ImGuiCol_Separator];
		colors[ImGuiCol_SeparatorHovered] = colors[ImGuiCol_Separator];

		colors[ImGuiCol_Tab] = colors[ImGuiCol_Button];
		colors[ImGuiCol_TabHovered] = colors[ImGuiCol_ButtonHovered];
		colors[ImGuiCol_TabActive] = colors[ImGuiCol_ButtonActive];
		colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
		colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
	}

	void ApplyDarkBlueTheme()
	{
		constexpr auto ColorFromBytes = [](uint8_t r, uint8_t g, uint8_t b)
			{
				return ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
			};

		auto& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		const ImVec4 bgColor = ColorFromBytes(37, 37, 38);
		const ImVec4 lightBgColor = ColorFromBytes(82, 82, 85);
		const ImVec4 veryLightBgColor = ColorFromBytes(90, 90, 95);

		const ImVec4 panelColor = ColorFromBytes(51, 51, 55);
		const ImVec4 panelHoverColor = ColorFromBytes(29, 151, 236);
		const ImVec4 panelActiveColor = ColorFromBytes(0, 119, 200);

		const ImVec4 textColor = ColorFromBytes(255, 255, 255);
		const ImVec4 textDisabledColor = ColorFromBytes(151, 151, 151);
		const ImVec4 borderColor = ColorFromBytes(78, 78, 78);

		colors[ImGuiCol_Text] = textColor;
		colors[ImGuiCol_TextDisabled] = textDisabledColor;
		colors[ImGuiCol_TextSelectedBg] = panelActiveColor;
		colors[ImGuiCol_WindowBg] = bgColor;
		colors[ImGuiCol_ChildBg] = bgColor;
		colors[ImGuiCol_PopupBg] = bgColor;
		colors[ImGuiCol_Border] = borderColor;
		colors[ImGuiCol_BorderShadow] = borderColor;
		colors[ImGuiCol_FrameBg] = panelColor;
		colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
		colors[ImGuiCol_FrameBgActive] = panelActiveColor;
		colors[ImGuiCol_TitleBg] = bgColor;
		colors[ImGuiCol_TitleBgActive] = bgColor;
		colors[ImGuiCol_TitleBgCollapsed] = bgColor;
		colors[ImGuiCol_MenuBarBg] = panelColor;
		colors[ImGuiCol_ScrollbarBg] = panelColor;
		colors[ImGuiCol_ScrollbarGrab] = lightBgColor;
		colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
		colors[ImGuiCol_ScrollbarGrabActive] = veryLightBgColor;
		colors[ImGuiCol_CheckMark] = panelActiveColor;
		colors[ImGuiCol_SliderGrab] = panelHoverColor;
		colors[ImGuiCol_SliderGrabActive] = panelActiveColor;
		colors[ImGuiCol_Button] = panelColor;
		colors[ImGuiCol_ButtonHovered] = panelHoverColor;
		colors[ImGuiCol_ButtonActive] = panelHoverColor;
		colors[ImGuiCol_Header] = panelColor;
		colors[ImGuiCol_HeaderHovered] = panelHoverColor;
		colors[ImGuiCol_HeaderActive] = panelActiveColor;
		colors[ImGuiCol_Separator] = borderColor;
		colors[ImGuiCol_SeparatorHovered] = borderColor;
		colors[ImGuiCol_SeparatorActive] = borderColor;
		colors[ImGuiCol_ResizeGrip] = bgColor;
		colors[ImGuiCol_ResizeGripHovered] = panelColor;
		colors[ImGuiCol_ResizeGripActive] = lightBgColor;
		colors[ImGuiCol_PlotLines] = panelActiveColor;
		colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
		colors[ImGuiCol_PlotHistogram] = panelActiveColor;
		colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
		colors[ImGuiCol_DragDropTarget] = bgColor;
		colors[ImGuiCol_NavHighlight] = bgColor;
		colors[ImGuiCol_DockingPreview] = panelActiveColor;
		colors[ImGuiCol_Tab] = bgColor;
		colors[ImGuiCol_TabActive] = panelActiveColor;
		colors[ImGuiCol_TabUnfocused] = bgColor;
		colors[ImGuiCol_TabUnfocusedActive] = panelActiveColor;
		colors[ImGuiCol_TabHovered] = panelHoverColor;

		style.WindowRounding = 0.0f;
		style.ChildRounding = 0.0f;
		style.FrameRounding = 0.0f;
		style.GrabRounding = 0.0f;
		style.PopupRounding = 0.0f;
		style.ScrollbarRounding = 0.0f;
		style.TabRounding = 0.0f;
	}

	ImGuiKey KeyCodeToImGuiKey(KeyCode code)
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

	bool ReadFile(std::string_view path, utl::vector<u8>& out)
	{
		std::ifstream file(path.data(), std::ios::binary);
		if (!file.is_open())
			return false;

		// Seek to the end to determine the file size
		file.seekg(0, std::ios::end);
		std::streamsize file_size = file.tellg();
		file.seekg(0, std::ios::beg);

		out.resize(file_size);

		if (!file.read(reinterpret_cast<char*>(out.data()), file_size))
			return false;

		return true;
	}

	EngineUI::EngineUI(Window& window) :
		window{ window }
	{
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		IMGUI_CHECKVERSION();
		IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

		io.BackendRendererUserData = (void*)this;
		io.BackendRendererName = "vulkan";
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
		//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can Create multi-viewports on the Renderer side (optional)

		// Load additional fonts
		auto& font_manager = ImFontManager::GetInstance();
		font_manager->SetFileReaderFn([](std::string_view path, utl::vector<u8>& out) { return ReadFile(path, out); });

		auto font_path = std::format("embed/font/{}", FONT_ICON_FILE_NAME_MDI);

		utl::vector<u8> data;
		ReadFile(font_path, data);

		// Load fonts
		static const ImWchar icons_ranges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };

		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphOffset.y = 1.0f;
		icons_config.OversampleH = icons_config.OversampleV = 1;
		icons_config.GlyphMinAdvanceX = 4.0f;
		icons_config.SizePixels = 12.0f;

		io.Fonts->AddFontDefault();
		font_manager->AddFont("small_icon", data, 13.f, &icons_config, icons_ranges);

		icons_config.GlyphOffset.y = 15.0f;
		icons_config.SizePixels = 48.0f;
		icons_config.GlyphMaxAdvanceX = 48.f;

		//font_manager->AddFont("big_icon", data, 48.f, &icons_config, icons_ranges);

		io.Fonts->Build();

		//ApplyModernStyle();
		ApplyCinderTheme();
		//ApplyDarkBlueTheme();
	}

	EngineUI::~EngineUI()
	{
		ImGui::EndFrame();
		ImGui::DestroyContext();
	}

	void EngineUI::NewFrame(float delta_time)
	{
		auto& extent = window.GetExtent();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(extent.width), static_cast<float>(extent.height));
		io.DeltaTime = delta_time;

		ImGui::NewFrame();
	}

	void EngineUI::OnAnyInput(const InputEvent& input_event)
	{

		switch (input_event.GetSource())
		{
		case EventSource::Keyboard:
		{
			const auto& key_input = static_cast<const KeyInputEvent&>(input_event);
			HandleKeyboardInput(key_input);
		} break;
		case EventSource::Character:
		{
			const auto& char_input = static_cast<const CharacterInputEvent&>(input_event);
			HandleCharacterInput(char_input);
		} break;
		case EventSource::Mouse:
		{
			const auto& mouse_input = static_cast<const MouseButtonInputEvent&>(input_event);
			HandleMouseInput(mouse_input);
		} break;
		case EventSource::Touchscreen:
		{
			const auto& touch_input = static_cast<const TouchInputEvent&>(input_event);
			HandleTouchInput(touch_input);
		} break;
		}
	}

	void EngineUI::HandleKeyboardInput(const KeyInputEvent& input_event)
	{
		auto action = input_event.GetAction();
		if (action != KeyAction::Down && action != KeyAction::Up)
			return;

		ImGuiIO& io = ImGui::GetIO();
		ImGuiKey imgui_key = KeyCodeToImGuiKey(input_event.GetCode());
		io.AddKeyEvent(imgui_key, (action == KeyAction::Down));
		//io.SetKeyEventNativeData(imgui_key, keycode, scancode);
	}

	void EngineUI::HandleCharacterInput(const CharacterInputEvent& input_event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(input_event.GetCharacterCode());
	}

	void EngineUI::HandleMouseInput(const MouseButtonInputEvent& input_event)
	{
		auto action = input_event.GetAction();
		switch (action)
		{
		case MouseAction::Down:
		case MouseAction::Up:
		{
			int button{ -1 };
			switch (input_event.GetButton())
			{
			case KeyCode::MouseLeft:
				button = 0;
				break;
			case KeyCode::MouseRight:
				button = 1;
				break;
			case KeyCode::MouseMiddle:
				button = 2;
				break;
			}

			ImGuiIO& io = ImGui::GetIO();
			if (button >= 0 && button < ImGuiMouseButton_COUNT)
				io.AddMouseButtonEvent(button, action == MouseAction::Down);
		} break;
		case MouseAction::Move:
		{
			ImGuiIO& io = ImGui::GetIO();

			// TODO: do update only when window is focused

			io.AddMousePosEvent(input_event.GetValueX(), input_event.GetValueY());
			last_mouse_x = input_event.GetValueX();
			last_mouse_y = input_event.GetValueY();
		} break;
		case MouseAction::Scroll:
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseWheelEvent(input_event.GetValueX(), input_event.GetValueY());
		} break;
		default:
			break;
		}
	}

	void EngineUI::HandleTouchInput(const TouchInputEvent& input_event)
	{

	}

	//void CImGuiEngineSystem::HandleFocusChanged(i32 focused)
//{
//	ImGuiIO& io = ImGui::GetIO();
//	io.AddFocusEvent(focused);
//}
//
//void CImGuiEngineSystem::HandleCursorEnter(i32 enter)
//{
//	ImGuiIO& io = ImGui::GetIO();
//	if (enter)
//	{
//		io.AddMousePosEvent(m_lastMouseX, m_lastMouseY);
//	}
//	else
//	{
//		m_lastMouseX = io.MousePos.x;
//		m_lastMouseY = io.MousePos.y;
//		io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
//	}
//}
}