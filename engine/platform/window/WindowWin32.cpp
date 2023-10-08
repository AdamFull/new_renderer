#include "WindowWin32.h"

using namespace engine::platform;

LRESULT CALLBACK CWindowWin32::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CWindowWin32* window = reinterpret_cast<CWindowWin32*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_CREATE:
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreate->lpCreateParams));
    } break;
    case WM_SIZE:
    {
        window->window_width = LOWORD(lParam);
        window->window_height = HIWORD(lParam);
    
        switch (wParam)
        {
        case SIZE_MINIMIZED: {
            window->window_state |= WINDOW_STATE_MINIMIZED_FLAG_BIT;
            window->window_state &= ~WINDOW_STATE_MAXIMIZED_FLAG_BIT;
        } break;
        case SIZE_MAXIMIZED: { 
            window->window_state |= WINDOW_STATE_MAXIMIZED_FLAG_BIT;
            window->window_state &= ~WINDOW_STATE_MINIMIZED_FLAG_BIT;
        } break;
        case SIZE_RESTORED:  { 
            window->window_state &= ~WINDOW_STATE_MINIMIZED_FLAG_BIT;
        }break;
        }
    
        window->window_state |= WINDOW_STATE_RESIZED_FLAG_BIT;
    } break;
    //case WM_SYSCOMMAND:
    //{
    //    switch (wParam)
    //    {
    //    case SC_MINIMIZE: { ShowWindow(hwnd, SW_MINIMIZE); } break;
    //    case SC_MAXIMIZE: { ShowWindow(hwnd, SW_MAXIMIZE); } break;
    //    case SC_RESTORE: { ShowWindow(hwnd, SW_RESTORE); } break;
    //    }
    //} break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

CWindowWin32::~CWindowWin32()
{
    destroy();
}

void CWindowWin32::create(const FWindowCreateInfo& window_ci)
{
    hinstance = GetModuleHandle(NULL);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hinstance;
    wc.lpszClassName = window_ci.name.c_str();

    RegisterClass(&wc);

    int32_t win_pos_x = window_ci.position_x;
    int32_t win_pos_y = window_ci.position_y;
    int32_t win_width = window_ci.width;
    int32_t win_height = window_ci.height;

    DWORD window_style_flags{ WS_OVERLAPPED };
    if ((window_ci.flags & WINDOW_BORDERLESS_FLAG_BIT) || (window_ci.flags & WINDOW_FULLSCREEN_FLAG_BIT))
    {
        window_style_flags |= WS_POPUP;
        if (window_ci.flags & WINDOW_FULLSCREEN_FLAG_BIT)
        {
            win_pos_x = win_pos_y = 0;
            win_width = GetSystemMetrics(SM_CXSCREEN);
            win_height = GetSystemMetrics(SM_CYSCREEN);
        }
    }

    if (window_ci.flags & WINDOW_RESIZABLE_FLAG_BIT)
        window_style_flags |= WS_TILEDWINDOW;
    if (window_ci.flags & WINDOW_VISIBLE_FLAG_BIT)
        window_style_flags |= WS_VISIBLE;
    if (window_ci.flags & WINDOW_DISABLED_FLAG_BIT)
        window_style_flags |= WS_DISABLED;

    hwnd = CreateWindowEx(
        0, wc.lpszClassName, wc.lpszClassName, window_style_flags,
        win_pos_x, win_pos_y, win_width, win_height,
        NULL, NULL, hinstance, this
    );

    int32_t show_window_mode{ SW_SHOWDEFAULT };
    if (window_ci.flags & WINDOW_SHOWN_FLAG_BIT)
        show_window_mode = SW_SHOW;
    else if (window_ci.flags & WINDOW_HIDDEN_FLAG_BIT)
        show_window_mode = SW_HIDE;
    else if(window_ci.flags & WINDOW_MINIMIZED_FLAG_BIT)
        show_window_mode = SW_SHOWMINIMIZED;
    else if (window_ci.flags & WINDOW_MAXIMIZED_FLAG_BIT)
        show_window_mode = SW_SHOWMAXIMIZED;

    ShowWindow(hwnd, show_window_mode);
}

void CWindowWin32::destroy()
{
    if (hwnd)
    {
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
}

bool CWindowWin32::process()
{
    if (window_state & WINDOW_STATE_RESIZED_FLAG_BIT)
        window_state &= ~WINDOW_STATE_RESIZED_FLAG_BIT;

    MSG msg = {};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT)
            return false;
    }

    return true;
}

void CWindowWin32::getWindowSize(int32_t& width, int32_t& height)
{
    width = window_width;
    height = window_height;
}

bool CWindowWin32::isResized()
{
    return window_state & WINDOW_STATE_RESIZED_FLAG_BIT;
}

bool CWindowWin32::isMinimized()
{
    return window_state & WINDOW_STATE_MINIMIZED_FLAG_BIT;
}

void CWindowWin32::getPlatformData(HINSTANCE& hinst, HWND& hwnd)
{
    hinst = this->hinstance;
    hwnd = this->hwnd;
}