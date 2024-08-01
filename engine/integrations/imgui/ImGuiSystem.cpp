#include "ImGuiSystem.h"

#include <engine/platform/window/WindowCallback.h>

#include "ImGuiElements.h"
#include "editor/HierarchyWindow.h"
#include "editor/InspectorWindow.h"
#include "ImFontManager.h"

#include <engine/integrations/imgui/font/IconsMaterialDesignIcons.h>

using namespace engine;
using namespace engine::render;

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

bool ReadFile(const std::string& path, utl::vector<u8>& out)
{
	std::ifstream file(path, std::ios::binary);
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

CImGuiEngineSystem::CImGuiEngineSystem(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CImGuiEngineSystem::Initialize()
{
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	IMGUI_CHECKVERSION();
	IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

	io.BackendRendererUserData = (void*)this;
	io.BackendRendererName = "vulkan";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
	//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can Create multi-viewports on the Renderer side (optional)
	
	auto& font_manager = CImFontManager::GetInstance();
	font_manager->SetFileReaderFn([](const std::string& path, utl::vector<u8>& out) { return ReadFile(path, out); });

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

	// Window events
	FWindowCallbackMapper::SubscribeFocusChange(this, &CImGuiEngineSystem::HandleFocusChanged);
	FWindowCallbackMapper::SubscribeCursorEnter(this, &CImGuiEngineSystem::HandleCursorEnter);
	FWindowCallbackMapper::SubscribeMousePosition(this, &CImGuiEngineSystem::HandleAxisInput);
	FWindowCallbackMapper::SubscribeMouseButton(this, &CImGuiEngineSystem::HandleMouseButton);
	FWindowCallbackMapper::SubscribeMouseScroll(this, &CImGuiEngineSystem::HandleMouseScroll);
	FWindowCallbackMapper::SubscribeKeyInput(this, &CImGuiEngineSystem::HandleKeyInput);
	FWindowCallbackMapper::SubscribeCharInput(this, &CImGuiEngineSystem::HandleCharInput);

	m_vWindows.emplace_back(std::make_unique<CHierarchyWindow>("Hierarchy"));
	m_vWindows.emplace_back(std::make_unique<CInspectorWindow>("Inspector"));

	for (auto& window : m_vWindows)
		window->Create();
}

void CImGuiEngineSystem::Update(f32 dt)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto resolution = render_context->GetScreenRenderRect();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(resolution.size.x, resolution.size.y);
	io.DeltaTime = dt;

	static int int_test_1{ 0 };
	static int int_test_2[2]{ 0 };
	static int int_test_3[3]{ 0 };
	static int int_test_4[4]{ 0 };

	static float float_test_1{ 0 };
	static float float_test_2[2]{ 0 };
	static float float_test_3[3]{ 0 };
	static float float_test_4[4]{ 0 };

	static double double_test_1{ 0 };
	static double double_test_2[2]{ 0 };
	static double double_test_3[3]{ 0 };
	static double double_test_4[4]{ 0 };

	ImGui::NewFrame();

	for (auto& window : m_vWindows)
		window->Draw(dt);

	//static bool show_demo_window{ true };
	//ImGui::ShowDemoWindow(&show_demo_window);

	//bool test_window_state{ true };
	//if (ImGui::Begin("Test window", &test_window_state))
	//{
	//	ImGui::GDragInt<1>("Drag int 1", &int_test_1, 1.f, -100, 100);
	//	ImGui::GDragInt<2>("Drag int 2", int_test_2, 1.f, -100, 100);
	//	ImGui::GDragInt<3>("Drag int 3", int_test_3, 1.f, -100, 100);
	//	ImGui::GDragInt<4>("Drag int 4", int_test_4, 1.f, -100, 100);
	//	ImGui::Separator();
	//	ImGui::GDragFloat<1>("Drag float 1", &float_test_1, 1.f, -1.f, 1.f);
	//	ImGui::GDragFloat<2>("Drag float 2", float_test_2, 1.f, -1.f, 1.f);
	//	ImGui::GDragFloat<3>("Drag float 3", float_test_3, 1.f, -1.f, 1.f);
	//	ImGui::GDragFloat<4>("Drag float 4", float_test_4, 1.f, -1.f, 1.f);
	//	ImGui::Separator();
	//	ImGui::GDragDouble<1>("Drag double 1", &double_test_1, 1.f, -100, 100);
	//	ImGui::GDragDouble<2>("Drag double 2", double_test_2, 1.f, -1.0, 1.0);
	//	ImGui::GDragDouble<3>("Drag double 3", double_test_3, 1.f, -1.0, 1.0);
	//	ImGui::GDragDouble<4>("Drag double 4", double_test_4, 1.f, -1.0, 1.0);
	//	ImGui::Separator();
	//	ImGui::Separator();
	//	ImGui::GSliderInt<1>("Slider int 1", &int_test_1, -100, 100);
	//	ImGui::GSliderInt<2>("Slider int 2", int_test_2, -100, 100);
	//	ImGui::GSliderInt<3>("Slider int 3", int_test_3, -100, 100);
	//	ImGui::GSliderInt<4>("Slider int 4", int_test_4, -100, 100);
	//	ImGui::Separator();
	//	ImGui::GSliderFloat<1>("Slider float 1", &float_test_1, -1.f, 1.f);
	//	ImGui::GSliderFloat<2>("Slider float 2", float_test_2, -1.f, 1.f);
	//	ImGui::GSliderFloat<3>("Slider float 3", float_test_3, -1.f, 1.f);
	//	ImGui::GSliderFloat<4>("Slider float 4", float_test_4, -1.f, 1.f);
	//	ImGui::Separator();
	//	ImGui::GSliderDouble<1>("Slider double 1", &double_test_1, -100, 100);
	//	ImGui::GSliderDouble<2>("Slider double 2", double_test_2, -1.0, 1.0);
	//	ImGui::GSliderDouble<3>("Slider double 3", double_test_3, -1.0, 1.0);
	//	ImGui::GSliderDouble<4>("Slider double 4", double_test_4, -1.0, 1.0);
	//	ImGui::Separator();
	//	ImGui::Separator();
	//	ImGui::GInputInt<1>("Input int 1", &int_test_1, 1, 10);
	//	ImGui::GInputInt<2>("Input int 2", int_test_2, 1, 10);
	//	ImGui::GInputInt<3>("Input int 3", int_test_3, 1, 10);
	//	ImGui::GInputInt<4>("Input int 4", int_test_4, 1, 10);
	//	ImGui::Separator();
	//	ImGui::GInputFloat<1>("Input float 1", &float_test_1, 0.01f, 1.f);
	//	ImGui::GInputFloat<2>("Input float 2", float_test_2, 0.01f, 1.f);
	//	ImGui::GInputFloat<3>("Input float 3", float_test_3, 0.01f, 1.f);
	//	ImGui::GInputFloat<4>("Input float 4", float_test_4, 0.01f, 1.f);
	//	ImGui::Separator();
	//	ImGui::GInputDouble<1>("Input double 1", &double_test_1, 0.005, 1.0);
	//	ImGui::GInputDouble<2>("Input double 2", double_test_2, 0.005, 1.0);
	//	ImGui::GInputDouble<3>("Input double 3", double_test_3, 0.005, 1.0);
	//	ImGui::GInputDouble<4>("Input double 4", double_test_4, 0.005, 1.0);
	//
	//	ImGui::End();
	//}

	ImGui::Render();
}

void CImGuiEngineSystem::PreRender()
{
}

void CImGuiEngineSystem::Render()
{

}

void CImGuiEngineSystem::Destroy()
{
	ImGui::DestroyContext();
}

void CImGuiEngineSystem::HandleFocusChanged(i32 focused)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddFocusEvent(focused);
}

void CImGuiEngineSystem::HandleCursorEnter(i32 enter)
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

void CImGuiEngineSystem::HandleAxisInput(f32 x, f32 y, f32 xmax, f32 ymax)
{
	ImGuiIO& io = ImGui::GetIO();

	// TODO: do update only when window is focused

	io.AddMousePosEvent(x, y);
	m_lastMouseX = x;
	m_lastMouseY = y;
}

void CImGuiEngineSystem::HandleMouseButton(EKeyCode code, EKeyState state)
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

void CImGuiEngineSystem::HandleMouseScroll(f32 x, f32 y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseWheelEvent(x, y);
}

void CImGuiEngineSystem::HandleKeyInput(EKeyCode code, EKeyState state)
{
	if (state != EKeyState::ePress && state != EKeyState::eRelease)
		return;

	ImGuiIO& io = ImGui::GetIO();
	ImGuiKey imgui_key = key_code_to_im_gui_key(code);
	io.AddKeyEvent(imgui_key, (state == EKeyState::ePress));
	//io.SetKeyEventNativeData(imgui_key, keycode, scancode);
}

void CImGuiEngineSystem::HandleCharInput(u32 character)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(character);
}