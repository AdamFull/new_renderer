#include "WindowCallback.h"

using namespace engine;

utl::vector<utl::reactive::function<void(int32_t, int32_t)>> FWindowCallbackMapper::sizeChangeCallbacks;
utl::vector<utl::reactive::function<void(int32_t)>> FWindowCallbackMapper::focusChangeCallbacks;
utl::vector<utl::reactive::function<void(int32_t)>> FWindowCallbackMapper::cursorEnterCallbacks;
utl::vector<utl::reactive::function<void(EKeyCode, EKeyState)>> FWindowCallbackMapper::mouseButtonCallbacks;
utl::vector<utl::reactive::function<void(float, float, float, float)>> FWindowCallbackMapper::mousePositionCallbacks;
utl::vector<utl::reactive::function<void(float, float)>> FWindowCallbackMapper::scrollCallbacks;
utl::vector<utl::reactive::function<void(EKeyCode, EKeyState)>> FWindowCallbackMapper::keyCallbacks;
utl::vector<utl::reactive::function<void(uint32_t)>> FWindowCallbackMapper::charCallbacks;
utl::vector<utl::reactive::function<void(int32_t)>> FWindowCallbackMapper::monitorCallbacks;

void FWindowCallbackMapper::windowSizeChanged(int32_t width, int32_t height)
{
	for (auto& callback : sizeChangeCallbacks)
		callback(width, height);
}

void FWindowCallbackMapper::windowInputFocusChanged(int32_t focused)
{
	for (auto& callback : focusChangeCallbacks)
		callback(focused);
}

void FWindowCallbackMapper::windowInputCursorEnter(int32_t entered)
{
	for (auto& callback : cursorEnterCallbacks)
		callback(entered);
}

void FWindowCallbackMapper::windowInputMouseButton(EKeyCode button, EKeyState action)
{
	for (auto& callback : mouseButtonCallbacks)
		callback(button, action);
}

void FWindowCallbackMapper::windowInputCursorPosition(double xpos, double ypos, double xmax, double ymax)
{
	for (auto& callback : mousePositionCallbacks)
		callback(static_cast<float>(xpos), static_cast<float>(ypos), static_cast<float>(xmax), static_cast<float>(ymax));
}

void FWindowCallbackMapper::windowInputScroll(double xoffset, double yoffset)
{
	for (auto& callback : scrollCallbacks)
		callback(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void FWindowCallbackMapper::windowInputKey(EKeyCode key, EKeyState action)
{
	for (auto& callback : keyCallbacks)
		callback(key, action);
}

void FWindowCallbackMapper::windowInputChar(uint32_t c)
{
	for (auto& callback : charCallbacks)
		callback(c);
}

void FWindowCallbackMapper::windowInputMonitor(int32_t monitor)
{
	for (auto& callback : monitorCallbacks)
		callback(monitor);
}