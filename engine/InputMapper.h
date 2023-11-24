#pragma once

#include <interface/WindowInterface.h>
#include <ufunction.hpp>
#include <udetail.hpp>

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
		std::unordered_map<EKeyCode, float> keyValue{};
		std::vector<utl::function<void(float)>> callbacks{};
	};

	struct FInputAction
	{
		std::vector<EKeyCode> keys{};
		EKeyState state{ EKeyState::eReleased };
		std::vector<utl::function<void(EKeyState)>> callbacks{};
	};

	class CInputMapper
	{
	public:
		void create();
		void update(float dt);

		template<class... _Args>
		void registerAction(const char* action_name, _Args&& ...args)
		{
			std::array<EKeyCode, sizeof...(_Args)> keys{ {args...} };
			
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
				input_action.callbacks.emplace_back(utl::function<void(EKeyState)>(std::forward<_Args>(args)...));
			}
		}

		template<class... _Args>
		void registerAxis(const char* axis_name, EAxisType type, _Args&& ...args)
		{
			static_assert(sizeof...(_Args) % 2ull == 0, "Incorrect argument count, it should be even.");

			auto& new_axis = m_inputAxis[utl::fnv1a_64_hash(axis_name)];
			new_axis.type = type;

			unpackAxisValues(new_axis.keyValue, std::forward<_Args>(args)...);
		}

		template<class... _Args>
		void bindAxis(const char* axis_name, _Args& ...args)
		{
			auto found = m_inputAxis.find(utl::fnv1a_64_hash(axis_name));
			if (found != m_inputAxis.end())
			{
				auto& found_axis = found->second;
				found_axis.callbacks.emplace_back(utl::function<void(float)>(std::forward<_Args>(args)...));
			}
		}

		//void registerAxisMapping(const char* name, )
	protected:
		void handleKeyInput(EKeyCode code, EKeyState state);
		void handleAxisInput(float x, float y, float xmax, float ymax);

		void unpackAxisValues(std::unordered_map<EKeyCode, float>& map) {}

		template<class... _Args>
		void unpackAxisValues(std::unordered_map<EKeyCode, float>& map, EKeyCode key, float value, _Args&&... args) 
		{ 
			map[key] = value; 
			unpackAxisValues(map, std::forward<_Args>(args)...);
		}

		template<class... _Args>
		void unpackAxisValues(std::unordered_map<EKeyCode, float>& map, _Args&&... args) { static_assert(true, "Incorrect argument types. Should be EKeyCode:float."); }
	private:
		CKeyDecoder m_keyDecoder{};

		std::unordered_map<size_t, FInputAction> m_inputActions{};
		std::unordered_map<size_t, FInputAxis> m_inputAxis{};

		std::unordered_map<EAxisType, float> m_hwAxis{};
	};
}