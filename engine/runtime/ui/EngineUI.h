#pragma once

#include <engine/runtime/InputEvents.h>

namespace engine
{
	class Window;

	class EngineUI final
	{
	public:
		EngineUI(Window& window);
		~EngineUI();

		void OnAnyInput(const InputEvent& input_event);

		void NewFrame(float delta_time);
	private:
		void HandleKeyboardInput(const KeyInputEvent& input_event);
		void HandleCharacterInput(const CharacterInputEvent& input_event);
		void HandleMouseInput(const MouseButtonInputEvent& input_event);
		void HandleTouchInput(const TouchInputEvent& input_event);

	private:
		Window& window;

		float last_mouse_x{};
		float last_mouse_y{};
	};
}