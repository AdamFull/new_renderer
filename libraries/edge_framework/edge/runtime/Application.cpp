#include "Application.h"

#include <cassert>

#include <imgui.h>

namespace edge
{
	bool Application::Prepare(const ApplicationOptions& options)
	{
		assert(options.window != nullptr && "Window must be valid");

		//auto& _debug_info = get_debug_info();
		//_debug_info.insert<field::MinMax, float>("fps", fps);
		//_debug_info.insert<field::MinMax, float>("frame_time", frame_time);

		lock_simulation_speed = options.benchmark_enabled;
		window = options.window;

		return true;
	}

	void Application::FixedUpdate(float delta_time)
	{

	}

	void Application::Update(float delta_time)
	{

	}

	void Application::Finish()
	{

	}

	bool Application::OnResize(uint32_t width, uint32_t height)
	{
		return true;
	}

	void Application::OnInputEvent(const InputEvent& input_event)
	{

	}

	void Application::SetName(std::string_view _name)
	{
		name = _name;
	}

	const std::string& Application::GetName() const
	{
		return name;
	}
}