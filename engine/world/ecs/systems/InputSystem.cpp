#include "InputSystem.h"

#include <engine/world/EntityManager.h>
#include <engine/world/ecs/components/BaseComponent.h>
#include <engine/world/ecs/components/CameraComponent.h>
#include <engine/world/ecs/components/TransformComponent.h>

#include <engine/profiler/Profiler.h>

constexpr const f32 movement_scale{ 50.f };

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
	TRACY_PROFILE_FUNCTION();

	//FWindowCallbackMapper::SubscribeKeyInput(this, &CInputSystem::keyInput);
	//FWindowCallbackMapper::SubscribeMousePosition(this, &CInputSystem::mouseMovementInput);
	//FWindowCallbackMapper::SubscribeMouseScroll(this, &CInputSystem::mouseWheelInput);
	//FWindowCallbackMapper::SubscribeMouseButton(this, &CInputSystem::keyInput);
}

void CInputSystem::__Update(f32 fDt)
{
	TRACY_PROFILE_FUNCTION();

	m_deltaTime = fDt;

	glm::vec3 orbitCenter{ 0.0f, 0.0f, 0.0f };
	f32 orbitDistance{ 10.0f };

	//if (m_keyDecoder.Test(EKeyCode::eKeyW)) movF = 1.f;
	//if (m_keyDecoder.Test(EKeyCode::eKeyS)) movF = -1.f;
	//if (m_keyDecoder.Test(EKeyCode::eKeyA)) movR = -1.f;
	//if (m_keyDecoder.Test(EKeyCode::eKeyD)) movR = 1.f;
	//if (m_keyDecoder.Test(EKeyCode::eKeySpace)) movU = 1.f;
	//if (m_keyDecoder.Test(EKeyCode::eKeyLCtrl)) movU = -1.f;
	//if (m_keyDecoder.Test(EKeyCode::eMouseMiddle))
	//	allowRotation = true;

	f32 dX = (curX - oldX) * fDt;
	oldX = curX;
	f32 dY = (curY - oldY) * fDt;
	oldY = curY;

	auto view = CEntityManager::View<FTransformComponent, FBaseComponent, FCameraComponent>();
	for (auto [entity, transform, base, camera] : view.each())
	{
		if (base.enabled)
		{
			if (allowRotation)
			{
				if (dX != 0.f || dY != 0.f)
				{
					dX *= movement_scale * 2.f;
					dY *= movement_scale * 2.f;

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

//void CInputSystem::keyInput(EKeyCode code, EKeyState state)
//{
//	m_keyDecoder.Update(code, state == EKeyState::ePress);
//}
//
//void CInputSystem::mouseMovementInput(f32 x, f32 y, f32 xmax, f32 ymax)
//{
//	curX = remap(x, 0.f, xmax, -1.f, 1.f);
//	curY = remap(y, 0.f, ymax, -1.f, 1.f);
//}
//
//void CInputSystem::mouseWheelInput(f32 x, f32 y)
//{
//
//}