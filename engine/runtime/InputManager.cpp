#include <engine/runtime/InputManager.h>

namespace engine
{
	std::vector<utl::reactive::function<void(const InputEvent&)>> InputManager::inputCallbacks;

	void InputManager::DispatchInputEvent(const InputEvent& input_event)
	{
		for (auto& callback : inputCallbacks)
			callback(input_event);
	}
}