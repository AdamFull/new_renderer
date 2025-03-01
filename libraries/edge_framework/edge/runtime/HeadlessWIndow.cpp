#include "HeadlessWIndow.h"

namespace edge
{
	HeadlessWindow::HeadlessWindow(const Window::Properties& properties) :
		Window(properties)
	{

	}

	auto HeadlessWindow::ShouldClose() const noexcept -> bool
	{
		return closed;
	}

	void HeadlessWindow::Close()
	{
		closed = true;
	}

	auto HeadlessWindow::GetDpiFactor() const noexcept -> float
	{
		return 1.0f;
	}

	void HeadlessWindow::SetTitle(std::string_view new_title) const
	{

	}

	auto HeadlessWindow::GetRequiredSurfaceExtensions() const -> std::vector<const char*>
	{
		return {};
	}
}