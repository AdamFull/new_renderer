#pragma once

#include <memory>
#include <string>

namespace engine
{
	enum EWindowFlagBits
	{
		WINDOW_SHOWN_FLAG_BIT = 1 << 0,
		WINDOW_HIDDEN_FLAG_BIT = 1 << 1,
		WINDOW_RESIZABLE_FLAG_BIT = 1 << 2,
		WINDOW_VISIBLE_FLAG_BIT = 1 << 3,
		WINDOW_DISABLED_FLAG_BIT = 1 << 4,

		WINDOW_MINIMIZED_FLAG_BIT = 1 << 5,
		WINDOW_MAXIMIZED_FLAG_BIT = 1 << 6,
		WINDOW_BORDERLESS_FLAG_BIT = 1 << 7,
		WINDOW_FULLSCREEN_FLAG_BIT = 1 << 8,
	};

	struct FWindowCreateInfo
	{
		std::string name{};
		uint32_t flags{ 0u };

		int32_t position_x{ 0 };
		int32_t position_y{ 0 };
		int32_t width{ 0 };
		int32_t height{ 0 };
	};

	class IWindowInterface
	{
	public:
		virtual ~IWindowInterface() = default;

		virtual void create(const FWindowCreateInfo& window_ci) = 0;
		virtual void destroy() = 0;

		virtual bool process() = 0;

		virtual void getWindowSize(int32_t& width, int32_t& height) = 0;
		virtual bool isResized() = 0;
		virtual bool isMinimized() = 0;
	};
}