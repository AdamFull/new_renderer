#pragma once

#include <reactive/function.h>
#include "InputEvents.h"

namespace engine
{
	class InputManager
	{
	public:
		static void DispatchInputEvent(const InputEvent& input_event);

		template <class... _Args>
		static void SubscribeInputEvent(_Args... args)
		{
			inputCallbacks.emplace_back(utl::reactive::function<void(const InputEvent&)>(std::forward<_Args>(args)...));
		}
	private:
		static std::vector<utl::reactive::function<void(const InputEvent&)>> inputCallbacks;
	};
} 