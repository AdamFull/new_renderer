#pragma once

#include <engine/interface/WindowInterface.h>
#include <Windows.h>

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

			void Create(const FWindowCreateInfo& window_ci) override;
			void Destroy() override;

			bool Process() override;

			void SetTitle(const std::string& title) override;

			void GetWindowSize(int32_t& width, int32_t& height) override;
			bool IsResized() override;
			bool IsMinimized() override;

			void GetPlatformData(HINSTANCE& hinst, HWND& hwnd);
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