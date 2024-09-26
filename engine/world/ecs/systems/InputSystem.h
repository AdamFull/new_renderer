#pragma once

#include <engine/runtime/InputEvents.h>
#include <engine/world/ecs/systems/BaseSystem.h>

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

				void OnInputEvent(const InputEvent& input_event) override;

				void __Create() override;
				void __Update(f32 fDt) override;

			protected:
				void HandleKeyboardInput(const KeyInputEvent& input_event);
				void HandleMouseInput(const MouseButtonInputEvent& input_event);
				void HandleTouchInput(const TouchInputEvent& input_event);
			private:
				CKeyDecoder m_keyDecoder{};
				f32 m_deltaTime{ 0.f };
				f32 oldX{ 0.f }, oldY{ 0.f }, curX{ 0.f }, curY{ 0.f };

				f32 movF{ 0.f }, movR{ 0.f }, movU{ 0.f };
				bool allowRotation{ false };
			};
		}
	}
}