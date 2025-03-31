#pragma once

#include "rt_window.h"

namespace edge {
	class HeadlessWindow final : public Window {
	public:
		HeadlessWindow(const Window::Properties& properties);
		virtual ~HeadlessWindow() = default;

		[[nodiscard]] auto should_close() const noexcept -> bool override;

		void close() override;

		[[nodiscard]] auto get_dpi_factor() const noexcept -> float override;

		void set_title(const std::string_view new_title) const override;

		[[nodiscard]] auto get_required_surface_extensions() const->std::vector<const char*> override;

	private:
		bool closed{ false };
	};
}