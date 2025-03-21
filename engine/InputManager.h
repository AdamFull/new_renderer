#pragma once

#include <reactive/function.h>
#include <edge/runtime/rt_input_events.h>

namespace engine
{
	class InputManager
	{
	public:
		static void DispatchInputEvent(const edge::InputEvent& input_event);

		template <class... _Args>
		static void SubscribeInputEvent(_Args... args)
		{
			inputCallbacks.emplace_back(utl::reactive::function<void(const edge::InputEvent&)>(std::forward<_Args>(args)...));
		}
	private:
		static std::vector<utl::reactive::function<void(const edge::InputEvent&)>> inputCallbacks;
	};
}