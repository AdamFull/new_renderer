#include "InputManager.h"

namespace engine
{
	std::vector<utl::reactive::function<void(const edge::InputEvent&)>> InputManager::inputCallbacks;

	void InputManager::DispatchInputEvent(const edge::InputEvent& input_event)
	{
		for (auto& callback : inputCallbacks)
			callback(input_event);
	}
}