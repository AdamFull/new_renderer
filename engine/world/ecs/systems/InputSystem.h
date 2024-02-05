#pragma once

#include <engine/interface/WindowInterface.h>
#include <engine/world/ecs/systems/BaseSystem.h>

#include <utility.hpp>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FInputAction
			{
				EKeyState state;
				stl::vector<utl::function<void(EKeyCode, EKeyState)>> listeners;
			};

			struct FInputAxis
			{
				stl::vector<utl::function<void(f32, f32)>> listeners;
			};

			class CInputSystem : public ISystem
			{
			public:
				CInputSystem(CSceneGraph* scenegraph);
				virtual ~CInputSystem() override = default;

				void __create() override;
				void __update(f32 fDt) override;

				//template <class... _Args>
				//void createAction(const char* action_name, _Args &&...args)
				//{
				//	// static_assert(stl::is_same_v<Args, EActionKey>&&..., "Type that you send to CreateAction is not supported.");
				//	stl::array<EKeyState, sizeof...(Args)> aKeys = { {args...} };
				//	for (auto key : aKeys)
				//	{
				//		m_mInputDescriptor.emplace(stl::make_pair(key, srActionName));
				//	}
				//}
			protected:
				void keyInput(EKeyCode code, EKeyState state);
				void mouseMovementInput(f32 x, f32 y, f32 xmax, f32 ymax);
				void mouseWheelInput(f32 x, f32 y);
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