#include "InputSystem.h"

#include "../../EntityManager.h"
#include "../components/BaseComponent.h"
#include "../components/CameraComponent.h"
#include "../components/TransformComponent.h"

#include <engine/InputManager.h>

#include <edge/util/utl_profiler.h>

constexpr const float mouse_scale{ 50.f };
constexpr const float movement_scale{ 2.f };

template<class _Ty>
inline _Ty remap(_Ty input, _Ty in_min, _Ty in_max, _Ty out_min, _Ty out_max)
{
	return (input - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

using namespace engine::world::ecs;

CInputSystem::CInputSystem()
{
}

void CInputSystem::__Create()
{
	InputManager::SubscribeInputEvent(this, &CInputSystem::OnAnyInput);
}

void CInputSystem::__Update(float fDt)
{
	EDGE_PROFILE_ZONE_SCOPED;

	m_deltaTime = fDt;

	glm::vec3 orbitCenter{ 0.0f, 0.0f, 0.0f };
	float orbitDistance{ 10.0f };

	if (key_decoder.test(edge::KeyCode::W)) movF = 1.f;
	if (key_decoder.test(edge::KeyCode::S)) movF = -1.f;
	if (key_decoder.test(edge::KeyCode::A)) movR = -1.f;
	if (key_decoder.test(edge::KeyCode::D)) movR = 1.f;
	if (key_decoder.test(edge::KeyCode::Space)) movU = 1.f;
	if (key_decoder.test(edge::KeyCode::LeftControl)) movU = -1.f;
	if (key_decoder.test(edge::KeyCode::MouseMiddle))
		allowRotation = true;

	float dX = (curX - oldX) * fDt;
	oldX = curX;
	float dY = (curY - oldY) * fDt;
	oldY = curY;

	auto view = EntityManager::View<FTransformComponent, FBaseComponent, FCameraComponent>();
	for (auto [entity, transform, base, camera] : view.each())
	{
		if (base.enabled)
		{
			if (allowRotation)
			{
				if (dX != 0.f || dY != 0.f)
				{
					dX *= mouse_scale;
					dY *= mouse_scale;

					glm::quat pitch = glm::angleAxis(-dY, camera.right);
					glm::quat yaw = glm::angleAxis(-dX, camera.up);

					glm::quat q = glm::normalize(glm::cross(pitch, yaw));
					transform.lrotation = q * transform.lrotation;

					camera.moved = true;
				}

				allowRotation = false;
			}

			if (movF != 0.f)
			{
				transform.lposition += camera.forward * movF * fDt * movement_scale;
				camera.moved = true;
			}

			if (movR != 0.f)
			{
				transform.lposition += camera.right * movR * fDt * movement_scale;
				camera.moved = true;
			}

			if (movU != 0.f)
			{
				transform.lposition += camera.up * movU * fDt * movement_scale;
				camera.moved = true;
			}

			movF = movR = movU = 0.f;

			break;
		}
	}
}

void CInputSystem::OnAnyInput(const edge::InputEvent& input_event)
{
	switch (input_event.get_source())
	{
	case edge::EventSource::Keyboard:
	{
		const auto& key_input = static_cast<const edge::KeyInputEvent&>(input_event);
		HandleKeyboardInput(key_input);
	} break;
	case edge::EventSource::Mouse:
	{
		const auto& mouse_input = static_cast<const edge::MouseButtonInputEvent&>(input_event);
		HandleMouseInput(mouse_input);
	} break;
	case edge::EventSource::Touchscreen:
	{
		const auto& touch_input = static_cast<const edge::TouchInputEvent&>(input_event);
		HandleTouchInput(touch_input);
	} break;
	}
}

void CInputSystem::HandleKeyboardInput(const edge::KeyInputEvent& input_event)
{
	key_decoder.update(input_event.get_code(), input_event.get_action() == edge::KeyAction::Down || input_event.get_action() == edge::KeyAction::Repeat);
}

void CInputSystem::HandleMouseInput(const edge::MouseButtonInputEvent& input_event)
{
	switch (input_event.get_action())
	{
	case edge::MouseAction::Down:
	case edge::MouseAction::Up:
	{
		key_decoder.update(input_event.get_button(), input_event.get_action() == edge::MouseAction::Down);
	} break;
	case edge::MouseAction::Move:
	{
		curX = remap(input_event.get_value_x(), 0.f, input_event.get_max_x(), -1.f, 1.f);
		curY = remap(input_event.get_value_y(), 0.f, input_event.get_max_y(), -1.f, 1.f);
	} break;
	case edge::MouseAction::Scroll:
	{

	} break;
	default:	
		break;
	}
}

void CInputSystem::HandleTouchInput(const edge::TouchInputEvent& input_event)
{

}