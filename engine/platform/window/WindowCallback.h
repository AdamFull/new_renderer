#pragma once

#include <reactive/function.h>
#include <engine/interface/WindowInterface.h>

namespace engine
{
	struct FWindowCallbackMapper
	{
        static void WindowSizeChanged(i32 width, i32 height);
        static void WindowInputFocusChanged(i32 focused);
        static void WindowInputCursorEnter(i32 entered);
        static void WindowInputMouseButton(EKeyCode button, EKeyState action);
        static void WindowInputCursorPosition(f64 xpos, f64 ypos, f64 xmax, f64 ymax);
        static void WindowInputScroll(f64 xoffset, f64 yoffset);
        static void WindowInputKey(EKeyCode key, EKeyState action);
        static void WindowInputChar(u32 c);
        static void WindowInputMonitor(i32 monitor);

        template <class... Args>
        static void WubscribeSizeChange(Args... args)
        {
            sizeChangeCallbacks.emplace_back(utl::reactive::function<void(i32, i32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void SubscribeFocusChange(Args... args)
        {
            focusChangeCallbacks.emplace_back(utl::reactive::function<void(i32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void SubscribeCursorEnter(Args... args)
        {
            cursorEnterCallbacks.emplace_back(utl::reactive::function<void(i32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void SubscribeMouseButton(Args... args)
        {
            mouseButtonCallbacks.emplace_back(utl::reactive::function<void(EKeyCode, EKeyState)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void SubscribeMousePosition(Args... args)
        {
            mousePositionCallbacks.emplace_back(utl::reactive::function<void(f32, f32, f32, f32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void SubscribeMouseScroll(Args... args)
        {
            scrollCallbacks.emplace_back(utl::reactive::function<void(f32, f32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void SubscribeKeyInput(Args... args)
        {
            keyCallbacks.emplace_back(utl::reactive::function<void(EKeyCode, EKeyState)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void SubscribeCharInput(Args... args)
        {
            charCallbacks.emplace_back(utl::reactive::function<void(u32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void SubscribeMonitor(Args... args)
        {
            monitorCallbacks.emplace_back(utl::reactive::function<void(i32)>(std::forward<Args>(args)...));
        }

    private:
        static utl::vector<utl::reactive::function<void(i32, i32)>> sizeChangeCallbacks;
        static utl::vector<utl::reactive::function<void(i32)>> focusChangeCallbacks;
        static utl::vector<utl::reactive::function<void(i32)>> cursorEnterCallbacks;
        static utl::vector<utl::reactive::function<void(EKeyCode, EKeyState)>> mouseButtonCallbacks;
        static utl::vector<utl::reactive::function<void(f32, f32, f32, f32)>> mousePositionCallbacks;
        static utl::vector<utl::reactive::function<void(f32, f32)>> scrollCallbacks;
        static utl::vector<utl::reactive::function<void(EKeyCode, EKeyState)>> keyCallbacks;
        static utl::vector<utl::reactive::function<void(u32)>> charCallbacks;
        static utl::vector<utl::reactive::function<void(i32)>> monitorCallbacks;
	};
}