#include "BaseSystem.h"

using namespace engine::world::ecs;

void ISystem::Create()
{
	m_stopWatch.stop<float>();
	__Create();
	m_createTime = m_stopWatch.stop<float>();
}

void ISystem::Update(float dt)
{
	m_stopWatch.stop<float>();
	__Update(dt);
	m_updateTime = m_stopWatch.stop<float>();
}