#pragma once

#include <engine/interface/WindowInterface.h>
#if defined(PLATFORM_WIN32)
#include <Windows.h>
#endif

class GLFWwindow;

namespace engine
{
	namespace platform
	{
		class CWindowGLFW : public IWindowInterface
		{
		public:
			CWindowGLFW();
			~CWindowGLFW() override;

			void create(const FWindowCreateInfo& window_ci) override;
			void destroy() override;

			bool process() override;

			void setTitle(const std::string& title) override;

			void getWindowSize(int32_t& width, int32_t& height) override;
			bool isResized() override;
			bool isMinimized() override;

			void windowSizeChanged();

#if defined(PLATFORM_WIN32)
			void getPlatformData(HINSTANCE& hinst, HWND& hwnd);
#endif
		private:
			bool m_sizeChanged{ false };
			GLFWwindow* m_pWindowHandle{ nullptr };
		};
	}
}