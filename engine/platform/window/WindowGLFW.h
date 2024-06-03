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

			void Create(const FWindowCreateInfo& window_ci) override;
			void Destroy() override;

			bool Process() override;

			void SetTitle(const std::string& title) override;

			void GetWindowSize(int32_t& width, int32_t& height) override;
			bool IsResized() override;
			bool IsMinimized() override;

			void WindowSizeChanged();

#if defined(PLATFORM_WIN32)
			void GetPlatformData(HINSTANCE& hinst, HWND& hwnd);
#endif
		private:
			bool m_sizeChanged{ false };
			GLFWwindow* m_pWindowHandle{ nullptr };
		};
	}
}