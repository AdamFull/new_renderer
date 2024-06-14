#include "EngineSystemManager.h"

using namespace engine;

void CEngineSystemManager::PreInitialize()
{
	for (auto& system : m_vSystems)
		system->PreInitialize();
}

void CEngineSystemManager::Initialize()
{
	for (auto& system : m_vSystems)
		system->Initialize();
}

void CEngineSystemManager::PostInitialize()
{
	for (auto& system : m_vSystems)
		system->PostInitialize();
}

void CEngineSystemManager::PreUpdate(f32 dt)
{
	for (auto& system : m_vSystems)
		system->PreUpdate(dt);
}

void CEngineSystemManager::Update(f32 dt)
{
	for (auto& system : m_vSystems)
		system->Update(dt);
}

void CEngineSystemManager::PostUpdate(f32 dt)
{
	for (auto& system : m_vSystems)
		system->PostUpdate(dt);
}

void CEngineSystemManager::PreRender()
{
	for (auto& system : m_vSystems)
		system->PreRender();
}

void CEngineSystemManager::Render()
{
	for (auto& system : m_vSystems)
		system->Render();
}

void CEngineSystemManager::PostRender()
{
	for (auto& system : m_vSystems)
		system->PostRender();
}

void CEngineSystemManager::PreDestroy()
{
	for (auto& system : m_vSystems)
		system->PreDestroy();
}

void CEngineSystemManager::Destroy()
{
	for (auto& system : m_vSystems)
		system->Destroy();
}

void CEngineSystemManager::PostDestroy()
{
	for (auto& system : m_vSystems)
		system->PostDestroy();
}