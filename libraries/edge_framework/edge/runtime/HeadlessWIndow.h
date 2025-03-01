#pragma once

#include "Window.h"

namespace edge
{
	class HeadlessWindow final : public Window
	{
	public:
		HeadlessWindow(const Window::Properties& properties);
		virtual ~HeadlessWindow() = default;

		[[nodiscard]] auto ShouldClose() const noexcept -> bool override;

		void Close() override;

		[[nodiscard]] auto GetDpiFactor() const noexcept -> float override;

		void SetTitle(const std::string_view new_title) const override;

		[[nodiscard]] auto GetRequiredSurfaceExtensions() const->std::vector<const char*> override;

	private:
		bool closed{ false };
	};
}