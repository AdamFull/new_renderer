#include "EngineUISystem.h"

#include "ImGuiElements.h"
#include "windows/HierarchyWindow.h"
#include "windows/InspectorWindow.h"

#include <engine/runtime/InputManager.h>

using namespace engine;

namespace engine
{
	

	EngineUISystem::EngineUISystem()
	{
	}

	void EngineUISystem::Initialize()
	{
		m_vWindows.emplace_back(std::make_unique<HierarchyWindow>("Hierarchy"));
		m_vWindows.emplace_back(std::make_unique<InspectorWindow>("Inspector"));

		for (auto& window : m_vWindows)
			window->Create();
	}

	void EngineUISystem::Update(f32 dt)
	{
		for (auto& window : m_vWindows)
			window->Draw(dt);
	}

	void EngineUISystem::Destroy()
	{
		ImGui::DestroyContext();
	}
}