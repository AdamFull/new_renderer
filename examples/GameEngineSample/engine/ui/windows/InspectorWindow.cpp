#include "InspectorWindow.h"

#include "../../world/EntityManager.h"
#include "../EditorState.h"
#include "../EditorTraits.h"
#include "../ImGuiElements.h"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

using namespace engine;
using namespace engine::world;

void InspectorWindow::Create()
{

}

void InspectorWindow::__draw(float fDt)
{
	auto& state = EditorState::GetInstance();
	//auto& imageLoader = graphics->getImageLoader();
	//auto& debug_draw = graphics->getDebugDraw();
	auto selected = state->GetSelection();

	// TODO: add replace component operation
	if (selected != entt::null)
	{
		auto& transform = EntityManager::GetComponent<ecs::FTransformComponent>(selected);
		auto& base = EntityManager::GetComponent<ecs::FBaseComponent>(selected);
		auto& hierarchy = EntityManager::GetComponent<ecs::FHierarchyComponent>(selected);

		if (ImGui::BeginPopup("add_component_popup"))
		{
			EntityManager::IterateRegisteredComponents([entity = selected](const FComponentMeta& meta) {
					if (!meta.has(entity)) {
						if (ImGui::MenuItem(meta.name.data()))
							EntityManager::AddComponent(entity, meta.type);
					}
				});

			ImGui::EndPopup();
		}

		ImGui::Separator();

		ImGui::GTextInput("Name", &base.name);
		ImGui::Separator();

		EntityManager::IterateComponents(selected, [&](const FComponentMeta& meta) {
				if (!meta.name.empty() && !(meta.flags & EComponentFlagBits::eHiddenInEditor)) {
					ImGui::PushID(meta.name.data());
					meta.edit(m_drawer, selected);
					ImGui::PopID();
				}
			});

		if (ImGui::Button(ICON_MDI_PLUS, ImVec2(-1.f, 0.f)))
			ImGui::OpenPopup("add_component_popup");

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				const char* path = (const char*)payload->Data;
				auto source = std::string(path);

				//if (fs::is_audio_format(source))
				//{
				//}
				//else if (fs::is_script_format(source))
				//{
				//}
			}
			ImGui::EndDragDropTarget();
		}
	}
}