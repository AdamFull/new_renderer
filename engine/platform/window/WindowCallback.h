#pragma once

#include <engine/interface/WindowInterface.h>
#include <ufunction.hpp>

namespace engine
{
	struct FWindowCallbackMapper
	{
        static void windowSizeChanged(int32_t width, int32_t height);
        static void windowInputFocusChanged(int32_t focused);
        static void windowInputCursorEnter(int32_t entered);
        static void windowInputMouseButton(EKeyCode button, EKeyState action);
        static void windowInputCursorPosition(double xpos, double ypos, double xmax, double ymax);
        static void windowInputScroll(double xoffset, double yoffset);
        static void windowInputKey(EKeyCode key, EKeyState action);
        static void windowInputChar(uint32_t c);
        static void windowInputMonitor(int32_t monitor);

        template <class... Args>
        static void subscribeSizeChange(Args... args)
        {
            sizeChangeCallbacks.emplace_back(utl::function<void(int32_t, int32_t)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeFocusChange(Args... args)
        {
            focusChangeCallbacks.emplace_back(utl::function<void(int32_t)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeCursorEnter(Args... args)
        {
            cursorEnterCallbacks.emplace_back(utl::function<void(int32_t)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMouseButton(Args... args)
        {
            mouseButtonCallbacks.emplace_back(utl::function<void(EKeyCode, EKeyState)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMousePosition(Args... args)
        {
            mousePositionCallbacks.emplace_back(utl::function<void(float, float, float, float)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMouseScroll(Args... args)
        {
            scrollCallbacks.emplace_back(utl::function<void(float, float)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeKeyInput(Args... args)
        {
            keyCallbacks.emplace_back(utl::function<void(EKeyCode, EKeyState)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeCharInput(Args... args)
        {
            charCallbacks.emplace_back(utl::function<void(uint32_t)>(std::forward<Args>(args)...));
        }

        template <class... Args>
        static void subscribeMonitor(Args... args)
        {
            monitorCallbacks.emplace_back(utl::function<void(int32_t)>(std::forward<Args>(args)...));
        }

    private:
        static std::vector<utl::function<void(int32_t, int32_t)>> sizeChangeCallbacks;
        static std::vector<utl::function<void(int32_t)>> focusChangeCallbacks;
        static std::vector<utl::function<void(int32_t)>> cursorEnterCallbacks;
        static std::vector<utl::function<void(EKeyCode, EKeyState)>> mouseButtonCallbacks;
        static std::vector<utl::function<void(float, float, float, float)>> mousePositionCallbacks;
        static std::vector<utl::function<void(float, float)>> scrollCallbacks;
        static std::vector<utl::function<void(EKeyCode, EKeyState)>> keyCallbacks;
        static std::vector<utl::function<void(uint32_t)>> charCallbacks;
        static std::vector<utl::function<void(int32_t)>> monitorCallbacks;
	};
}