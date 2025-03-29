#pragma once

#include <edge/runtime/rt_input_events.h>
#include "BaseSystem.h"

#include <functional>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FInputAxis
			{
				std::vector<std::function<void(float, float)>> listeners;
			};

			class CInputSystem : public ISystem
			{
			public:
				CInputSystem();
				virtual ~CInputSystem() override = default;

				void __Create() override;
				void __Update(float fDt) override;

			protected:
				void OnAnyInput(const edge::InputEvent& input_event);
				void HandleKeyboardInput(const edge::KeyInputEvent& input_event);
				void HandleMouseInput(const edge::MouseButtonInputEvent& input_event);
				void HandleTouchInput(const edge::TouchInputEvent& input_event);
			private:
				edge::KeyDecoder key_decoder{};
				float m_deltaTime{ 0.f };
				float oldX{ 0.f }, oldY{ 0.f }, curX{ 0.f }, curY{ 0.f };

				float movF{ 0.f }, movR{ 0.f }, movU{ 0.f };
				bool allowRotation{ false };
			};
		}
	}
}