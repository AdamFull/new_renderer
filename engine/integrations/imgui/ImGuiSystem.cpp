#include "ImGuiSystem.h"

#include <engine/platform/window/WindowCallback.h>

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

CImGuiEngineSystem::CImGuiEngineSystem(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CImGuiEngineSystem::Initialize()
{
	// Window events
	FWindowCallbackMapper::SubscribeFocusChange(this, &CImGuiEngineSystem::HandleFocusChanged);
	FWindowCallbackMapper::SubscribeCursorEnter(this, &CImGuiEngineSystem::HandleCursorEnter);
	FWindowCallbackMapper::SubscribeMousePosition(this, &CImGuiEngineSystem::HandleAxisInput);
	FWindowCallbackMapper::SubscribeMouseButton(this, &CImGuiEngineSystem::HandleMouseButton);
	FWindowCallbackMapper::SubscribeMouseScroll(this, &CImGuiEngineSystem::HandleMouseScroll);
	FWindowCallbackMapper::SubscribeKeyInput(this, &CImGuiEngineSystem::HandleKeyInput);
	FWindowCallbackMapper::SubscribeCharInput(this, &CImGuiEngineSystem::HandleCharInput);
}

void CImGuiEngineSystem::Update(f32 dt)
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