#pragma once

#include <engine/interface/WindowInterface.h>
#include <utility.hpp>

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
		stl::unordered_map<EKeyCode, f32> keyValue{};
		stl::vector<utl::function<void(f32)>> callbacks{};
	};

	struct FInputAction
	{
		stl::vector<EKeyCode> keys{};
		EKeyState state{ EKeyState::eReleased };
		stl::vector<utl::function<void(EKeyState)>> callbacks{};
	};

	class CInputMapper
	{
	public:
		void create();
		void update(f32 dt);

		template<class... _Args>
		void registerAction(const char* action_name, _Args&& ...args)
		{
			stl::array<EKeyCode, sizeof...(_Args)> keys{ {args...} };
			
			auto& action = m_inputActions[utl::fnv1a_64_hash(action_name)];
			action.keys.insert(action.keys.end(), keys.begin(), keys.end());
		}

		template<class... _Args>
		void bindAction(const char* action_name, _Args&& ...args)
		{
			auto found = m_inputActions.find(utl::fnv1a_64_hash(action_name));
			if (found != m_inputActions.end())
			{
				auto& input_action = found->second;
				input_action.callbacks.emplace_back(utl::function<void(EKeyState)>(stl::forward<_Args>(args)...));
			}
		}

		template<class... _Args>
		void registerAxis(const char* axis_name, EAxisType type, _Args&& ...args)
		{
			static_assert(sizeof...(_Args) % 2ull == 0, "Incorrect argument count, it should be even.");

			auto& new_axis = m_inputAxis[utl::fnv1a_64_hash(axis_name)];
			new_axis.type = type;

			unpackAxisValues(new_axis.keyValue, stl::forward<_Args>(args)...);
		}

		template<class... _Args>
		void bindAxis(const char* axis_name, _Args& ...args)
		{
			auto found = m_inputAxis.find(utl::fnv1a_64_hash(axis_name));
			if (found != m_inputAxis.end())
			{
				auto& found_axis = found->second;
				found_axis.callbacks.emplace_back(utl::function<void(f32)>(stl::forward<_Args>(args)...));
			}
		}

		//void registerAxisMapping(const char* name, )
	protected:
		void handleKeyInput(EKeyCode code, EKeyState state);
		void handleAxisInput(f32 x, f32 y, f32 xmax, f32 ymax);

		void unpackAxisValues(stl::unordered_map<EKeyCode, f32>& map) {}

		template<class... _Args>
		void unpackAxisValues(stl::unordered_map<EKeyCode, f32>& map, EKeyCode key, f32 value, _Args&&... args) 
		{ 
			map[key] = value; 
			unpackAxisValues(map, stl::forward<_Args>(args)...);
		}

		template<class... _Args>
		void unpackAxisValues(stl::unordered_map<EKeyCode, f32>& map, _Args&&... args) { static_assert(true, "Incorrect argument types. Should be EKeyCode:f32."); }
	private:
		CKeyDecoder m_keyDecoder{};

		stl::unordered_map<u64, FInputAction> m_inputActions{};
		stl::unordered_map<u64, FInputAxis> m_inputAxis{};

		stl::unordered_map<EAxisType, f32> m_hwAxis{};
	};
}