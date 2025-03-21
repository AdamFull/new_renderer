#pragma once

#include <reactive/function.h>
#include <edge/util/utl_hash.hpp>
#include <edge/runtime/rt_input_events.h>

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
		std::unordered_map<edge::KeyCode, f32> keyValue{};
		std::vector<utl::reactive::function<void(f32)>> callbacks{};
	};

	struct FInputAction
	{
		std::vector<edge::KeyCode> keys{};
		//KeyState state{ KeyState::eReleased };
		//std::vector<utl::reactive::function<void(KeyState)>> callbacks{};
	};

	class CInputMapper
	{
	public:
		void Create();
		void Update(f32 dt);

		void on_input_event(const edge::InputEvent& input_event);

		template<class... _Args>
		void RegisterAction(const char* action_name, _Args&& ...args)
		{
			std::array<edge::KeyCode, sizeof...(_Args)> keys{ {args...} };
			
			auto& action = m_inputActions[edge::murmur_hash_64(action_name)];
			action.keys.insert(action.keys.end(), keys.begin(), keys.end());
		}

		template<class... _Args>
		void BindAction(const char* action_name, _Args&& ...args)
		{
			auto found = m_inputActions.find(edge::murmur_hash_64(action_name));
			if (found != m_inputActions.end())
			{
				auto& input_action = found->second;
				//input_action.callbacks.emplace_back(utl::reactive::function<void(EKeyState)>(std::forward<_Args>(args)...));
			}
		}

		template<class... _Args>
		void RegisterAxis(const char* axis_name, EAxisType type, _Args&& ...args)
		{
			static_assert(sizeof...(_Args) % 2ull == 0, "Incorrect argument count, it should be even.");

			auto& new_axis = m_inputAxis[edge::murmur_hash_64(axis_name)];
			new_axis.type = type;

			UnpackAxisValues(new_axis.keyValue, std::forward<_Args>(args)...);
		}

		template<class... _Args>
		void BindAxis(const char* axis_name, _Args& ...args)
		{
			auto found = m_inputAxis.find(edge::murmur_hash_64(axis_name));
			if (found != m_inputAxis.end())
			{
				auto& found_axis = found->second;
				found_axis.callbacks.emplace_back(utl::reactive::function<void(f32)>(std::forward<_Args>(args)...));
			}
		}

		//void registerAxisMapping(const char* name, )
	protected:
		//void HandleKeyInput(KeyCode code, EKeyState state);
		//void HandleAxisInput(f32 x, f32 y, f32 xmax, f32 ymax);

		void UnpackAxisValues(std::unordered_map<edge::KeyCode, f32>& map) {}

		template<class... _Args>
		void UnpackAxisValues(std::unordered_map<edge::KeyCode, f32>& map, edge::KeyCode key, f32 value, _Args&&... args)
		{ 
			map[key] = value; 
			UnpackAxisValues(map, std::forward<_Args>(args)...);
		}

		template<class... _Args>
		void UnpackAxisValues(std::unordered_map<edge::KeyCode, f32>& map, _Args&&... args) { static_assert(true, "Incorrect argument types. Should be EKeyCode:f32."); }
	private:
		edge::KeyDecoder m_keyDecoder{};

		std::unordered_map<uint64_t, FInputAction> m_inputActions{};
		std::unordered_map<uint64_t, FInputAxis> m_inputAxis{};

		std::unordered_map<EAxisType, f32> m_hwAxis{};
	};
}