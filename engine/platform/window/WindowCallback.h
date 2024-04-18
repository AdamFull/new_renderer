#pragma once

#include <reactive/function.h>
#include <engine/interface/WindowInterface.h>

namespace engine
{
	struct FWindowCallbackMapper
	{
        static void windowSizeChanged(i32 width, i32 height);
        static void windowInputFocusChanged(i32 focused);
        static void windowInputCursorEnter(i32 entered);
        static void windowInputMouseButton(EKeyCode button, EKeyState action);
        static void windowInputCursorPosition(f64 xpos, f64 ypos, f64 xmax, f64 ymax);
        static void windowInputScroll(f64 xoffset, f64 yoffset);
        static void windowInputKey(EKeyCode key, EKeyState action);
        static void windowInputChar(u32 c);
        static void windowInputMonitor(i32 monitor);

        template <class... Args>
        static void subscribeSizeChange(Args... args)
        {
            sizeChangeCallbacks.emplace_back(utl::reactive::function<void(i32, i32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeFocusChange(Args... args)
        {
            focusChangeCallbacks.emplace_back(utl::reactive::function<void(i32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeCursorEnter(Args... args)
        {
            cursorEnterCallbacks.emplace_back(utl::reactive::function<void(i32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMouseButton(Args... args)
        {
            mouseButtonCallbacks.emplace_back(utl::reactive::function<void(EKeyCode, EKeyState)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMousePosition(Args... args)
        {
            mousePositionCallbacks.emplace_back(utl::reactive::function<void(f32, f32, f32, f32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMouseScroll(Args... args)
        {
            scrollCallbacks.emplace_back(utl::reactive::function<void(f32, f32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeKeyInput(Args... args)
        {
            keyCallbacks.emplace_back(utl::reactive::function<void(EKeyCode, EKeyState)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeCharInput(Args... args)
        {
            charCallbacks.emplace_back(utl::reactive::function<void(u32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMonitor(Args... args)
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