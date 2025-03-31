#pragma once

#include <edge/runtime/rt_input_events.h>

namespace edge
{
	class Window;
}

namespace engine
{
	class EngineUI final
	{
	public:
		EngineUI(edge::Window& window);
		~EngineUI();

		void OnAnyInput(const edge::InputEvent& input_event);

		void NewFrame(float delta_time);
	private:
		void HandleKeyboardInput(const edge::KeyInputEvent& input_event);
		void HandleCharacterInput(const edge::CharacterInputEvent& input_event);
		void HandleMouseInput(const edge::MouseButtonInputEvent& input_event);
		void HandleTouchInput(const edge::TouchInputEvent& input_event);

	private:
		edge::Window& window;

		float last_mouse_x{};
		float last_mouse_y{};
	};
}