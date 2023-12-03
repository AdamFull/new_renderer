#pragma once

#include <engine/interface/WindowInterface.h>
#include <engine/world/ecs/systems/BaseSystem.h>

#include <ufunction.hpp>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FInputAction
			{
				EKeyState state;
				std::vector<utl::function<void(EKeyCode, EKeyState)>> listeners;
			};

			struct FInputAxis
			{
				std::vector<utl::function<void(float, float)>> listeners;
			};

			class CInputSystem : public ISystem
			{
			public:
				CInputSystem(CSceneGraph* scenegraph);
				virtual ~CInputSystem() override = default;

				void __create() override;
				void __update(float fDt) override;

				//template <class... _Args>
				//void createAction(const char* action_name, _Args &&...args)
				//{
				//	// static_assert(std::is_same_v<Args, EActionKey>&&..., "Type that you send to CreateAction is not supported.");
				//	std::array<EKeyState, sizeof...(Args)> aKeys = { {args...} };
				//	for (auto key : aKeys)
				//	{
				//		m_mInputDescriptor.emplace(std::make_pair(key, srActionName));
				//	}
				//}
			protected:
				void keyInput(EKeyCode code, EKeyState state);
				void mouseMovementInput(float x, float y, float xmax, float ymax);
				void mouseWheelInput(float x, float y);
			private:
				CKeyDecoder m_keyDecoder{};
				float m_deltaTime{ 0.f };
				float oldX{ 0.f }, oldY{ 0.f }, curX{ 0.f }, curY{ 0.f };

				float movF{ 0.f }, movR{ 0.f }, movU{ 0.f };
				bool allowRotation{ false };
			};
		}
	}
}