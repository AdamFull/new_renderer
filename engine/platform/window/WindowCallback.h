#pragma once

#include <engine/interface/WindowInterface.h>
#include <utility.hpp>

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
            sizeChangeCallbacks.emplace_back(utl::function<void(i32, i32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeFocusChange(Args... args)
        {
            focusChangeCallbacks.emplace_back(utl::function<void(i32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeCursorEnter(Args... args)
        {
            cursorEnterCallbacks.emplace_back(utl::function<void(i32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMouseButton(Args... args)
        {
            mouseButtonCallbacks.emplace_back(utl::function<void(EKeyCode, EKeyState)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMousePosition(Args... args)
        {
            mousePositionCallbacks.emplace_back(utl::function<void(f32, f32, f32, f32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMouseScroll(Args... args)
        {
            scrollCallbacks.emplace_back(utl::function<void(f32, f32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeKeyInput(Args... args)
        {
            keyCallbacks.emplace_back(utl::function<void(EKeyCode, EKeyState)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeCharInput(Args... args)
        {
            charCallbacks.emplace_back(utl::function<void(u32)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMonitor(Args... args)
        {
            monitorCallbacks.emplace_back(utl::function<void(i32)>(std::forward<Args>(args)...));
        }

    private:
        static std::vector<utl::function<void(i32, i32)>> sizeChangeCallbacks;
        static std::vector<utl::function<void(i32)>> focusChangeCallbacks;
        static std::vector<utl::function<void(i32)>> cursorEnterCallbacks;
        static std::vector<utl::function<void(EKeyCode, EKeyState)>> mouseButtonCallbacks;
        static std::vector<utl::function<void(f32, f32, f32, f32)>> mousePositionCallbacks;
        static std::vector<utl::function<void(f32, f32)>> scrollCallbacks;
        static std::vector<utl::function<void(EKeyCode, EKeyState)>> keyCallbacks;
        static std::vector<utl::function<void(u32)>> charCallbacks;
        static std::vector<utl::function<void(i32)>> monitorCallbacks;
	};
}