#include "rt_headless_wIndow.h"

namespace edge {
	HeadlessWindow::HeadlessWindow(const Window::Properties& properties) :
		Window(properties) {
	}

	auto HeadlessWindow::should_close() const noexcept -> bool {
		return closed;
	}

	void HeadlessWindow::close() {
		closed = true;
	}

	auto HeadlessWindow::get_dpi_factor() const noexcept -> float {
		return 1.0f;
	}

	void HeadlessWindow::set_title(std::string_view new_title) const {

	}

	auto HeadlessWindow::get_required_surface_extensions() const -> std::vector<const char*> {
		return {};
	}
}