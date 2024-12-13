#pragma once

#include <edge/runtime/InputEvents.h>
#include "BaseSystem.h"

#include <reactive/function.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FInputAxis
			{
				utl::vector<utl::reactive::function<void(f32, f32)>> listeners;
			};

			class CInputSystem : public ISystem
			{
			public:
				CInputSystem();
				virtual ~CInputSystem() override = default;

				void __Create() override;
				void __Update(f32 fDt) override;

			protected:
				void OnAnyInput(const edge::InputEvent& input_event);
				void HandleKeyboardInput(const edge::KeyInputEvent& input_event);
				void HandleMouseInput(const edge::MouseButtonInputEvent& input_event);
				void HandleTouchInput(const edge::TouchInputEvent& input_event);
			private:
				edge::CKeyDecoder key_decoder{};
				f32 m_deltaTime{ 0.f };
				f32 oldX{ 0.f }, oldY{ 0.f }, curX{ 0.f }, curY{ 0.f };

				f32 movF{ 0.f }, movR{ 0.f }, movU{ 0.f };
				bool allowRotation{ false };
			};
		}
	}
}