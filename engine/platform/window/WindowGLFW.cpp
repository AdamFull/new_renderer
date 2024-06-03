#include "WindowGLFW.h"

#if defined(PLATFORM_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "WindowCallback.h"

using namespace engine;
using namespace engine::platform;

namespace key_converter
{
    static EKeyCode glfw_key_to_engine_code(int32_t scancode)
    {
        switch (scancode)
        {
        case GLFW_KEY_A: return EKeyCode::eKeyA;
        case GLFW_KEY_B: return EKeyCode::eKeyB;
        case GLFW_KEY_C: return EKeyCode::eKeyC;
        case GLFW_KEY_D: return EKeyCode::eKeyD;
        case GLFW_KEY_E: return EKeyCode::eKeyE;
        case GLFW_KEY_F: return EKeyCode::eKeyF;
        case GLFW_KEY_G: return EKeyCode::eKeyG;
        case GLFW_KEY_H: return EKeyCode::eKeyH;
        case GLFW_KEY_I: return EKeyCode::eKeyI;
        case GLFW_KEY_J: return EKeyCode::eKeyJ;
        case GLFW_KEY_K: return EKeyCode::eKeyK;
        case GLFW_KEY_L: return EKeyCode::eKeyL;
        case GLFW_KEY_M: return EKeyCode::eKeyM;
        case GLFW_KEY_N: return EKeyCode::eKeyN;
        case GLFW_KEY_O: return EKeyCode::eKeyO;
        case GLFW_KEY_P: return EKeyCode::eKeyP;
        case GLFW_KEY_Q: return EKeyCode::eKeyQ;
        case GLFW_KEY_R: return EKeyCode::eKeyR;
        case GLFW_KEY_S: return EKeyCode::eKeyS;
        case GLFW_KEY_T: return EKeyCode::eKeyT;
        case GLFW_KEY_U: return EKeyCode::eKeyU;
        case GLFW_KEY_V: return EKeyCode::eKeyV;
        case GLFW_KEY_W: return EKeyCode::eKeyW;
        case GLFW_KEY_X: return EKeyCode::eKeyX;
        case GLFW_KEY_Y: return EKeyCode::eKeyY;
        case GLFW_KEY_Z: return EKeyCode::eKeyZ;

        case GLFW_KEY_1: return EKeyCode::eKey1;
        case GLFW_KEY_2: return EKeyCode::eKey2;
        case GLFW_KEY_3: return EKeyCode::eKey3;
        case GLFW_KEY_4: return EKeyCode::eKey4;
        case GLFW_KEY_5: return EKeyCode::eKey5;
        case GLFW_KEY_6: return EKeyCode::eKey6;
        case GLFW_KEY_7: return EKeyCode::eKey7;
        case GLFW_KEY_8: return EKeyCode::eKey8;
        case GLFW_KEY_9: return EKeyCode::eKey9;
        case GLFW_KEY_0: return EKeyCode::eKey0;
            
        case GLFW_KEY_ENTER: return EKeyCode::eKyeReturn;
        case GLFW_KEY_ESCAPE: return EKeyCode::eKyeEscape;
        case GLFW_KEY_BACKSPACE: return EKeyCode::eKyeBackspace;
        case GLFW_KEY_TAB: return EKeyCode::eKyeTab;
        case GLFW_KEY_SPACE: return EKeyCode::eKeySpace;
        case GLFW_KEY_LEFT_CONTROL: return EKeyCode::eKeyLCtrl;
        case GLFW_KEY_RIGHT_CONTROL: return EKeyCode::eKeyRCtrl;
        case GLFW_KEY_LEFT_SHIFT: return EKeyCode::eKeyLShift;
        case GLFW_KEY_RIGHT_SHIFT: return EKeyCode::eKeyRShift;
            
        case GLFW_KEY_MINUS: return EKeyCode::eKeyMinus;
        case GLFW_KEY_EQUAL: return EKeyCode::eKeyEquals;
        case GLFW_KEY_LEFT_BRACKET: return EKeyCode::eKeyLeftBracket;
        case GLFW_KEY_RIGHT_BRACKET: return EKeyCode::eKeyRightBracket;
        case GLFW_KEY_BACKSLASH: return EKeyCode::eKeyBackslash;
        case GLFW_KEY_SEMICOLON: return EKeyCode::eKeySemicolon;
        case GLFW_KEY_APOSTROPHE: return EKeyCode::eKeyApostrophe;
        case GLFW_KEY_COMMA: return EKeyCode::eKeyComma;
        case GLFW_KEY_PERIOD: return EKeyCode::eKeyPeriod;
        case GLFW_KEY_SLASH: return EKeyCode::eKeySlash;
        case GLFW_KEY_CAPS_LOCK: return EKeyCode::eKeyCapslock;

        case GLFW_KEY_F1: return EKeyCode::eKeyF1;
        case GLFW_KEY_F2: return EKeyCode::eKeyF2;
        case GLFW_KEY_F3: return EKeyCode::eKeyF3;
        case GLFW_KEY_F4: return EKeyCode::eKeyF4;
        case GLFW_KEY_F5: return EKeyCode::eKeyF5;
        case GLFW_KEY_F6: return EKeyCode::eKeyF6;
        case GLFW_KEY_F7: return EKeyCode::eKeyF7;
        case GLFW_KEY_F8: return EKeyCode::eKeyF8;
        case GLFW_KEY_F9: return EKeyCode::eKeyF9;
        case GLFW_KEY_F10: return EKeyCode::eKeyF10;
        case GLFW_KEY_F11: return EKeyCode::eKeyF11;
        case GLFW_KEY_F12: return EKeyCode::eKeyF12;

        case GLFW_KEY_PRINT_SCREEN: return EKeyCode::eKeyPrintscreen;
        case GLFW_KEY_SCROLL_LOCK: return EKeyCode::eKeyScrollLock;
        case GLFW_KEY_PAUSE: return EKeyCode::eKeyPause;
        case GLFW_KEY_INSERT: return EKeyCode::eKeyInsert;

        case GLFW_KEY_HOME: return EKeyCode::eKeyHome;
        case GLFW_KEY_PAGE_UP: return EKeyCode::eKeyPageUp;
        case GLFW_KEY_DELETE: return EKeyCode::eKeyDelete;
        case GLFW_KEY_END: return EKeyCode::eKeyEnd;
        case GLFW_KEY_PAGE_DOWN: return EKeyCode::eKeyPageDown;
        case GLFW_KEY_RIGHT: return EKeyCode::eKeyRight;
        case GLFW_KEY_LEFT: return EKeyCode::eKeyLeft;
        case GLFW_KEY_DOWN: return EKeyCode::eKeyDown;
        case GLFW_KEY_UP: return EKeyCode::eKeyUp;
        default: return EKeyCode::eUnknown;
        }
    }

    static EKeyCode glfw_gamepad_btn_to_engine_code(int32_t code)
    {
        switch (code)
        {
        case GLFW_GAMEPAD_BUTTON_A: return EKeyCode::eButtonA; break;
        case GLFW_GAMEPAD_BUTTON_B: return EKeyCode::eButtonB; break;
        case GLFW_GAMEPAD_BUTTON_X: return EKeyCode::eButtonX; break;
        case GLFW_GAMEPAD_BUTTON_Y: return EKeyCode::eButtonY; break;
        case GLFW_GAMEPAD_BUTTON_BACK: return EKeyCode::eButtonBack; break;
        case GLFW_GAMEPAD_BUTTON_GUIDE: return EKeyCode::eButtonGuide; break;
        case GLFW_GAMEPAD_BUTTON_START: return EKeyCode::eButtonStart; break;
        case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: return EKeyCode::eButtonLeftStick; break;
        case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: return EKeyCode::eButtonRightStick; break;
        case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: return EKeyCode::eButtonLeftStick; break;
        case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return EKeyCode::eButtonRightShoulder; break;
        case GLFW_GAMEPAD_BUTTON_DPAD_UP: return EKeyCode::eButtonDPADUp; break;
        case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: return EKeyCode::eButtonDPADDown; break;
        case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: return EKeyCode::eButtonDPADLeft; break;
        case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: return EKeyCode::eButtonDPADRight; break;
        }

        return EKeyCode::eUnknown;
    }

    static EKeyCode glfw_mouse_key_to_engine_code(int32_t code)
    {
        switch (code)
        {
        case GLFW_MOUSE_BUTTON_LEFT: return EKeyCode::eMouseLeft; break;
        case GLFW_MOUSE_BUTTON_MIDDLE: return EKeyCode::eMouseMiddle; break;
        case GLFW_MOUSE_BUTTON_RIGHT: return EKeyCode::eMouseRight; break;
        case GLFW_MOUSE_BUTTON_4: return EKeyCode::eMouseX1; break;
        case GLFW_MOUSE_BUTTON_5: return EKeyCode::eMouseX2; break;
        }

        return EKeyCode::eUnknown;
    }

    static EKeyState glfw_key_state_to_engine_code(int32_t code)
    {
        switch (code)
        {
        case GLFW_PRESS: return EKeyState::ePress;
        case GLFW_RELEASE: return EKeyState::eRelease;
        }
    }
}

namespace window_callbacks
{
	void window_size_changed_cb(GLFWwindow* window, int32_t width, int32_t height)
	{
		auto* handle = static_cast<CWindowGLFW*>(glfwGetWindowUserPointer(window));
        handle->WindowSizeChanged();

        FWindowCallbackMapper::WindowSizeChanged(width, height);
	}

	void window_input_focus_changed_cb(GLFWwindow* window, int32_t focused)
	{
        FWindowCallbackMapper::WindowInputFocusChanged(focused);
	}

	void window_input_cursor_enter_cb(GLFWwindow* window, int32_t entered)
	{
        FWindowCallbackMapper::WindowInputCursorEnter(entered);
	}

	void window_input_mouse_button_cb(GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
	{
        FWindowCallbackMapper::WindowInputMouseButton(key_converter::glfw_mouse_key_to_engine_code(button), key_converter::glfw_key_state_to_engine_code(action));
	}

	void window_input_cursor_position_cb(GLFWwindow* window, double xpos, double ypos)
	{
        int32_t width{}, height{};
        glfwGetWindowSize(window, &width, &height);

        FWindowCallbackMapper::WindowInputCursorPosition(xpos, ypos, static_cast<double>(width), static_cast<double>(height));
	}

	void window_input_scroll_cb(GLFWwindow* window, double xoffset, double yoffset)
	{
        FWindowCallbackMapper::WindowInputScroll(xoffset, yoffset);
	}

	void window_input_key_cb(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
	{
        FWindowCallbackMapper::WindowInputKey(key_converter::glfw_key_to_engine_code(key), key_converter::glfw_key_state_to_engine_code(action));
	}

	void window_input_char_cb(GLFWwindow* window, uint32_t c)
	{
        FWindowCallbackMapper::WindowInputChar(c);
	}

	void window_input_monitor_callback(GLFWmonitor* window, int32_t monitor)
	{
		//auto* handle = static_cast<CWindowGLFW*>(glfwGetWindowUserPointer(window));
	}
}

CWindowGLFW::CWindowGLFW()
{
	// TODO: handle error
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

CWindowGLFW::~CWindowGLFW()
{
	Destroy();
}

void CWindowGLFW::Create(const FWindowCreateInfo& window_ci)
{
	auto* monitor = glfwGetPrimaryMonitor();

	int32_t width{ window_ci.width }, height{ window_ci.height };
	if (window_ci.flags & WINDOW_FULLSCREEN_FLAG_BIT)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		width = mode->width;
		height = mode->height;
	}

	if(window_ci.flags & WINDOW_RESIZABLE_FLAG_BIT)
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	if (window_ci.flags & WINDOW_MAXIMIZED_FLAG_BIT)
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    if (window_ci.flags & WINDOW_FULLSCREEN_FLAG_BIT)
	    m_pWindowHandle = glfwCreateWindow(width, height, window_ci.name.c_str(), monitor, nullptr);
    else
        m_pWindowHandle = glfwCreateWindow(width, height, window_ci.name.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(m_pWindowHandle, this);
	glfwSetInputMode(m_pWindowHandle, GLFW_STICKY_KEYS, GLFW_TRUE);

	if (window_ci.flags & WINDOW_FULLSCREEN_FLAG_BIT)
		glfwSetInputMode(m_pWindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetWindowSizeCallback(m_pWindowHandle, &window_callbacks::window_size_changed_cb);
	glfwSetWindowFocusCallback(m_pWindowHandle, &window_callbacks::window_input_focus_changed_cb);
	glfwSetCursorEnterCallback(m_pWindowHandle, &window_callbacks::window_input_cursor_enter_cb);
	glfwSetCursorPosCallback(m_pWindowHandle, &window_callbacks::window_input_cursor_position_cb);
	glfwSetMouseButtonCallback(m_pWindowHandle, &window_callbacks::window_input_mouse_button_cb);
	glfwSetScrollCallback(m_pWindowHandle, &window_callbacks::window_input_scroll_cb);
	glfwSetKeyCallback(m_pWindowHandle, &window_callbacks::window_input_key_cb);
	glfwSetCharCallback(m_pWindowHandle, &window_callbacks::window_input_char_cb);
	glfwSetMonitorCallback(&window_callbacks::window_input_monitor_callback);
}

void CWindowGLFW::Destroy()
{
	if(m_pWindowHandle)
		glfwDestroyWindow(m_pWindowHandle);
	glfwTerminate();
}

bool CWindowGLFW::Process()
{
    m_sizeChanged = false;
	bool should_close = !glfwWindowShouldClose(m_pWindowHandle);
	glfwPollEvents();
	return should_close;
}

void CWindowGLFW::SetTitle(const std::string& title)
{
    glfwSetWindowTitle(m_pWindowHandle, title.c_str());
}

void CWindowGLFW::GetWindowSize(int32_t& width, int32_t& height)
{
	glfwGetWindowSize(m_pWindowHandle, &width, &height);
}

bool CWindowGLFW::IsResized()
{
    return m_sizeChanged;
}

bool CWindowGLFW::IsMinimized()
{
	return glfwGetWindowAttrib(m_pWindowHandle, GLFW_ICONIFIED);
}

void CWindowGLFW::WindowSizeChanged()
{
    m_sizeChanged = true;
}

#if defined(PLATFORM_WIN32)
void CWindowGLFW::GetPlatformData(HINSTANCE& hinst, HWND& hwnd)
{
	hwnd = glfwGetWin32Window(m_pWindowHandle);
	hinst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
}
#endif