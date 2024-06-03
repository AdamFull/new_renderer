#pragma once

#include <engine/interface/WindowInterface.h>
#include <reactive/function.h>
#include <utils/hash.h>
#include <container/hash_map.h>

namespace engine
{
	enum class EAxisType
	{
		eMouseX,
		eMouseY,
		eMouseScrollX,
		eMouseScrollY,
		eMouseOldX,
		eMouseOldY,
		eMouseDeltaX,
		eMouseDeltaY,
		eKeyAction
	};

	struct FInputAxis
	{
		EAxisType type{};
		utl::hash_map<EKeyCode, f32> keyValue{};
		utl::vector<utl::reactive::function<void(f32)>> callbacks{};
	};

	struct FInputAction
	{
		utl::vector<EKeyCode> keys{};
		EKeyState state{ EKeyState::eReleased };
		utl::vector<utl::reactive::function<void(EKeyState)>> callbacks{};
	};

	class CInputMapper
	{
	public:
		void Create();
		void Update(f32 dt);

		template<class... _Args>
		void RegisterAction(const char* action_name, _Args&& ...args)
		{
			std::array<EKeyCode, sizeof...(_Args)> keys{ {args...} };
			
			auto& action = m_inputActions[utl::fnv1a_64_hash(action_name)];
			action.keys.insert(action.keys.end(), keys.begin(), keys.end());
		}

		template<class... _Args>
		void BindAction(const char* action_name, _Args&& ...args)
		{
			auto found = m_inputActions.find(utl::fnv1a_64_hash(action_name));
			if (found != m_inputActions.end())
			{
				auto& input_action = found->second;
				input_action.callbacks.emplace_back(utl::reactive::function<void(EKeyState)>(std::forward<_Args>(args)...));
			}
		}

		template<class... _Args>
		void RegisterAxis(const char* axis_name, EAxisType type, _Args&& ...args)
		{
			static_assert(sizeof...(_Args) % 2ull == 0, "Incorrect argument count, it should be even.");

			auto& new_axis = m_inputAxis[utl::fnv1a_64_hash(axis_name)];
			new_axis.type = type;

			UnpackAxisValues(new_axis.keyValue, std::forward<_Args>(args)...);
		}

		template<class... _Args>
		void BindAxis(const char* axis_name, _Args& ...args)
		{
			auto found = m_inputAxis.find(utl::fnv1a_64_hash(axis_name));
			if (found != m_inputAxis.end())
			{
				auto& found_axis = found->second;
				found_axis.callbacks.emplace_back(utl::reactive::function<void(f32)>(std::forward<_Args>(args)...));
			}
		}

		//void registerAxisMapping(const char* name, )
	protected:
		void HandleKeyInput(EKeyCode code, EKeyState state);
		void HandleAxisInput(f32 x, f32 y, f32 xmax, f32 ymax);

		void UnpackAxisValues(utl::hash_map<EKeyCode, f32>& map) {}

		template<class... _Args>
		void UnpackAxisValues(utl::hash_map<EKeyCode, f32>& map, EKeyCode key, f32 value, _Args&&... args) 
		{ 
			map[key] = value; 
			UnpackAxisValues(map, std::forward<_Args>(args)...);
		}

		template<class... _Args>
		void UnpackAxisValues(utl::hash_map<EKeyCode, f32>& map, _Args&&... args) { static_assert(true, "Incorrect argument types. Should be EKeyCode:f32."); }
	private:
		CKeyDecoder m_keyDecoder{};

		utl::hash_map<u64, FInputAction> m_inputActions{};
		utl::hash_map<u64, FInputAxis> m_inputAxis{};

		utl::hash_map<EAxisType, f32> m_hwAxis{};
	};
}