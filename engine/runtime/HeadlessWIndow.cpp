#include "HeadlessWIndow.h"

namespace engine
{
	HeadlessWindow::HeadlessWindow(const Window::Properties& properties) :
		Window(properties)
	{

	}

	vk::SurfaceKHR HeadlessWindow::CreateSurface(vk::Instance instance, vk::AllocationCallbacks* callbacks)
	{
		return nullptr;
	}

	bool HeadlessWindow::ShouldClose()
	{
		return closed;
	}

	void HeadlessWindow::Close()
	{
		closed = true;
	}

	float HeadlessWindow::GetDpiFactor() const
	{
		return 1.0f;
	}

	void HeadlessWindow::SetTitle(std::string_view new_title)
	{

	}

	std::vector<const char*> HeadlessWindow::GetRequiredSurfaceExtensions() const
	{
		return {};
	}
}