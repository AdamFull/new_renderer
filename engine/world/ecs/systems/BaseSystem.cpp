#include "BaseSystem.h"

using namespace engine::world::ecs;

void ISystem::create()
{
	m_stopWatch.stop<float>();
	__create();
	m_createTime = m_stopWatch.stop<float>();
}

void ISystem::update(float dt)
{
	m_stopWatch.stop<float>();
	__update(dt);
	m_updateTime = m_stopWatch.stop<float>();
}