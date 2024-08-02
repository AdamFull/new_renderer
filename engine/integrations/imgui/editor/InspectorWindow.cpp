#include "InspectorWindow.h"

#include <engine/world/EntityManager.h>
#include <engine/integrations/imgui/EditorState.h>
#include <engine/integrations/imgui/EditorTraits.h>
#include <engine/integrations/imgui/ImGuiElements.h>

#include <engine/filesystem/vfs_helper.h>
#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

using namespace engine;
using namespace engine::filesystem;
using namespace engine::world;

void CInspectorWindow::Create()
{

}

void CInspectorWindow::__draw(float fDt)
{
	auto& state = CEditorState::GetInstance();
	//auto& imageLoader = graphics->getImageLoader();
	//auto& debug_draw = graphics->getDebugDraw();
	auto selected = state->GetSelection();

	// TODO: add replace component operation
	if (selected != entt::null)
	{
		auto& transform = CEntityManager::GetComponent<ecs::FTransformComponent>(selected);
		auto& base = CEntityManager::GetComponent<ecs::FBaseComponent>(selected);
		auto& hierarchy = CEntityManager::GetComponent<ecs::FHierarchyComponent>(selected);

		if (ImGui::BeginPopup("add_component_popup"))
		{
			CEntityManager::IterateRegisteredComponents([entity = selected](const auto& type, const auto& meta)
				{
					if (!CEntityManager::HasComponent(entity, type))
					{
						if (ImGui::MenuItem(meta.name))
							CEntityManager::AddComponent(entity, type);
					}
				});

			ImGui::EndPopup();
		}

		ImGui::Separator();

		ImGui::GTextInput("Name", &base.name);
		ImGui::Separator();

		CEntityManager::IterateComponents(selected, [](ecs::IComponent* component) 
			{
				auto* name = component->GetName();
				if (name)
				{
					ImGui::PushID(name);
					if (ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap))
					{
						component->OnEdit();
						ImGui::Separator();
					}
					ImGui::PopID();
				}
			});

		if (ImGui::Button(ICON_MDI_PLUS, ImVec2(-1.f, 0.f)))
			ImGui::OpenPopup("add_component_popup");

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const char* path = (const char*)payload->Data;
				auto source = std::string(path);

				if (fs::is_audio_format(source))
				{
					//if (!CEntityManager::TryGetComponent<ecs::FAudioComponent>(selected))
					//{
					//	ecs::FAudioComponent naudio{};
					//	naudio.source = source;
					//
					//	CEntityManager::AddComponent<ecs::FAudioComponent>(selected, std::move(naudio));
					//}
				}
				else if (fs::is_script_format(source))
				{
					//if (!CEntityManager::TryGetComponent<ecs::FScriptComponent>(selected))
					//{
					//	ecs::FScriptComponent nscript{};
					//	nscript.source = source;
					//
					//	CEntityManager::AddComponent<ecs::FScriptComponent>(selected, std::move(nscript));
					//}
				}
			}
			ImGui::EndDragDropTarget();
		}
	}
}