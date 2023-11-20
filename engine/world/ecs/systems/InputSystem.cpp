#include "InputSystem.h"

#include <world/SceneGraph.h>

#include <world/ecs/components/BaseComponent.h>
#include <world/ecs/components/CameraComponent.h>
#include <world/ecs/components/TransformComponent.h>

#include <platform/window/WindowCallback.h>

constexpr const float movement_scale{ 50.f };

template<class _Ty>
inline _Ty remap(_Ty input, _Ty in_min, _Ty in_max, _Ty out_min, _Ty out_max)
{
	return (input - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

using namespace engine::world::ecs;

CInputSystem::CInputSystem(CSceneGraph* scenegraph)
{
	m_pSceneGraph = scenegraph;
}

void CInputSystem::__create()
{
	FWindowCallbackMapper::subscribeKeyInput(this, &CInputSystem::keyInput);
	FWindowCallbackMapper::subscribeMousePosition(this, &CInputSystem::mouseMovementInput);
	FWindowCallbackMapper::subscribeMouseScroll(this, &CInputSystem::mouseWheelInput);
	FWindowCallbackMapper::subscribeMouseButton(this, &CInputSystem::keyInput);
}

void CInputSystem::__update(float fDt)
{
	m_deltaTime = fDt;

	auto& registry = m_pSceneGraph->getRegistry();
	auto view = registry->view<FTransformComponent, FBaseComponent, FCameraComponent>();
	for (auto [entity, transform, base, camera] : view.each())
	{
		if (base.enabled)
		{
			float dX = (curX - oldX) * fDt;
			oldX = curX;
			float dY = (curY - oldY) * fDt;
			oldY = curY;

			if (dX != 0.f || dY != 0.f)
			{
				dX *= movement_scale;
				dY *= movement_scale;

				glm::quat pitch = glm::angleAxis(-dY, camera.right);
				glm::quat yaw = glm::angleAxis(-dX, camera.up);
				glm::quat q = glm::normalize(glm::cross(pitch, yaw));

				transform.lrotation = q * transform.lrotation;

				camera.moved = true;
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

void CInputSystem::keyInput(EKeyCode code, EKeyState state)
{
	auto& registry = m_pSceneGraph->getRegistry();

	auto view = registry->view<FTransformComponent, FBaseComponent, FCameraComponent>();
	for (auto [entity, transform, base, camera] : view.each())
	{
		if (base.enabled && state == EKeyState::ePress)
		{
			if(code == EKeyCode::eKeyW) movF = 1.f;
			if(code == EKeyCode::eKeyS) movF = -1.f;
			if(code == EKeyCode::eKeyA) movR = -1.f;
			if(code == EKeyCode::eKeyD) movR = 1.f;
			if(code == EKeyCode::eKeySpace) movU = 1.f;
			if(code == EKeyCode::eKeyLCtrl) movU = -1.f;

			break;
		}
	}
}

void CInputSystem::mouseMovementInput(float x, float y, float xmax, float ymax)
{
	curX = remap(x, 0.f, xmax, -1.f, 1.f);
	curY = remap(y, 0.f, ymax, -1.f, 1.f);
}

void CInputSystem::mouseWheelInput(float x, float y)
{

}