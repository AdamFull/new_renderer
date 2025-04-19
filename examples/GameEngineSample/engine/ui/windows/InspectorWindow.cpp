#include "InspectorWindow.h"

#include "../../world/EntityManager.h"
#include "../EditorState.h"
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
		auto& transform = EntityManager::get_component<ecs::TransformComponent>(selected);
		auto& base = EntityManager::get_component<ecs::BaseComponent>(selected);
		auto& hierarchy = EntityManager::get_component<ecs::HierarchyComponent>(selected);

		if (ImGui::BeginPopup("add_component_popup"))
		{
			EntityManager::iterate_registered_components(
				[entity = selected](const ComponentMeta& meta) {
					if (!meta.has(entity)) {
						if (ImGui::MenuItem(meta.name.data())) {
							EntityManager::add_component(entity, meta.type);
						}
					}
				});

			ImGui::EndPopup();
		}

		ImGui::Separator();

		ImGui::GTextInput("Name", &base.name);
		ImGui::Separator();

		EntityManager::iterate_components(selected,
			[&](const ComponentMeta& meta) {
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