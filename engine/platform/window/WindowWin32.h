#pragma once

#include <engine/interface/WindowInterface.h>
#define NOMINMAX
#include <Windows.h>
#undef min
#undef max

namespace engine
{
	namespace platform
	{
		enum EWindowStateFlagBits : uint32_t
		{
			WINDOW_STATE_MINIMIZED_FLAG_BIT = 1 << 0,
			WINDOW_STATE_MAXIMIZED_FLAG_BIT = 1 << 1,
			WINDOW_STATE_RESIZED_FLAG_BIT = 1 << 2,
		};

		class CWindowWin32 : public IWindowInterface
		{
		public:
			CWindowWin32();
			~CWindowWin32() override;

			void create(const FWindowCreateInfo& window_ci) override;
			void destroy() override;

			bool process() override;

			void setTitle(const std::string& title) override;

			void getWindowSize(int32_t& width, int32_t& height) override;
			bool isResized() override;
			bool isMinimized() override;

			void getPlatformData(HINSTANCE& hinst, HWND& hwnd);
		protected:
			static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		private:
			HINSTANCE hinstance{};
			HWND hwnd{};

			int32_t window_width{}, window_height{};
			uint32_t window_state{ 0u };
		};
	}
}