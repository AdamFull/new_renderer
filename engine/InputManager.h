#pragma once

#include <utility>
#include <functional>
#include <edge/runtime/rt_input_events.h>

namespace engine
{
	class InputManager
	{
	public:
		static void DispatchInputEvent(const edge::InputEvent& input_event);

		template <typename _T, typename _Func>
		static void SubscribeInputEvent(_T* instance, _Func memberFunc) {
			inputCallbacks.emplace_back(
				[instance, memberFunc](const edge::InputEvent& event) {
					(instance->*memberFunc)(event);
				});
		}

		template<typename _F>
		static void SubscribeInputEvent(_F&& f) {
			inputCallbacks.emplace_back(std::forward<_F>(f));
		}
	private:
		static std::vector<std::function<void(const edge::InputEvent&)>> inputCallbacks;
	};
}