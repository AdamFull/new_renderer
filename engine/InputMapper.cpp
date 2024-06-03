#include "InputMapper.h"

#include <engine/platform/window/WindowCallback.h>

using namespace engine;

template<class _Ty>
inline _Ty remap(_Ty input, _Ty in_min, _Ty in_max, _Ty out_min, _Ty out_max)
{
	return (input - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void CInputMapper::Create()
{
	FWindowCallbackMapper::SubscribeKeyInput(this, &CInputMapper::HandleKeyInput);
	FWindowCallbackMapper::SubscribeMousePosition(this, &CInputMapper::HandleAxisInput);
	//FWindowCallbackMapper::SubscribeMouseScroll(this, &CInputMapper::handleMovementInput);
	FWindowCallbackMapper::SubscribeMouseButton(this, &CInputMapper::HandleKeyInput);
}

void CInputMapper::Update(float dt)
{
	{
		auto& mouse_old_x = m_hwAxis[EAxisType::eMouseOldX];
		auto& mouse_cur_x = m_hwAxis[EAxisType::eMouseX];

		m_hwAxis[EAxisType::eMouseDeltaX] = (mouse_cur_x - mouse_old_x) * dt;
		mouse_old_x = mouse_cur_x;


		auto& mouse_old_y = m_hwAxis[EAxisType::eMouseOldY];
		auto& mouse_cur_y = m_hwAxis[EAxisType::eMouseY];

		m_hwAxis[EAxisType::eMouseDeltaY] = (mouse_cur_y - mouse_old_y) * dt;
		mouse_old_y = mouse_cur_y;
	}

	// Process input actions
	for (auto& [name, action] : m_inputActions)
	{
		// Test for pressed keys
		bool is_passes{ true };
		for (const auto& key : action.keys)
			is_passes = is_passes && m_keyDecoder.Test(key);

		if (is_passes)
		{
			switch (action.state)
			{
			case EKeyState::eReleased: action.state = EKeyState::ePress; break;
			case EKeyState::ePress: action.state = EKeyState::ePressed; break;
			}
		}
		else
		{
			switch (action.state)
			{
			case EKeyState::ePressed: action.state = EKeyState::eRelease; is_passes = true; break;
			case EKeyState::eRelease: action.state = EKeyState::eReleased; is_passes = true; break;
			}
		}

		if (is_passes)
		{
			for (auto& callback : action.callbacks)
				callback(action.state);
		}
	}

	// Process input axis
	for (auto& [name, axis] : m_inputAxis)
	{
		if (axis.type == EAxisType::eKeyAction)
		{
			for (auto& [key, value] : axis.keyValue)
			{
				float axis_value{ 0.f };
				if (m_keyDecoder.Test(key))
					axis_value = value;

				for (auto& callback : axis.callbacks)
					callback(value);
			}
		}
		else
		{
			for (auto& hwaxis : m_hwAxis)
			{
				switch (axis.type)
				{
				case EAxisType::eMouseX: break;
				case EAxisType::eMouseY: break;
				case EAxisType::eMouseScrollX: break;
				case EAxisType::eMouseScrollY: break;
				case EAxisType::eMouseOldX: break;
				case EAxisType::eMouseOldY: break;
				case EAxisType::eMouseDeltaX: break;
				case EAxisType::eMouseDeltaY: break;
				}
			}
		}	
	}
}

void CInputMapper::HandleKeyInput(EKeyCode code, EKeyState state)
{
	m_keyDecoder.Update(code, state == EKeyState::ePress);
}

void CInputMapper::HandleAxisInput(float x, float y, float xmax, float ymax)
{
	m_hwAxis[EAxisType::eMouseX] = remap(x, 0.f, xmax, -1.f, 1.f);
	m_hwAxis[EAxisType::eMouseY] = remap(y, 0.f, ymax, -1.f, 1.f);
}