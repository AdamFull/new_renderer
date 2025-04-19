#pragma once

#include <edge/runtime/rt_input_events.h>
#include "BaseSystem.h"

#include <functional>

namespace engine::world::ecs {
	struct InputAxis {
		std::vector<std::function<void(float, float)>> listeners;
	};

	class InputSystem final : public ISystem {
	public:
		InputSystem();
		virtual ~InputSystem() override = default;

		void __create() override;
		void __update(float fDt) override;

	protected:
		void on_any_input(const edge::InputEvent& input_event);
		void handle_keyboard_input(const edge::KeyInputEvent& input_event);
		void handle_mouse_input(const edge::MouseButtonInputEvent& input_event);
		void handle_touch_input(const edge::TouchInputEvent& input_event);
	private:
		edge::KeyDecoder key_decoder{};
		float m_deltaTime{ 0.f };
		float oldX{ 0.f }, oldY{ 0.f }, curX{ 0.f }, curY{ 0.f };

		float movF{ 0.f }, movR{ 0.f }, movU{ 0.f };
		bool allowRotation{ false };
	};
}