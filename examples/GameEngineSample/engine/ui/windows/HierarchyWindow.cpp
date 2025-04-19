#include "HierarchyWindow.h"

#include <imgui.h>
#include "../../world/EntityManager.h"

#include "../EditorTraits.h"
#include "../EditorState.h"

using namespace engine;
using namespace engine::world;

void HierarchyWindow::__draw(float fDt)
{
    //auto& actionBuffer = EGEditor->getActionBuffer();
    auto root = EntityManager::GetRoot();
    auto current_size = ImGui::GetWindowSize();

    BuildHierarchy(root);

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
    {
        if (ImGui::IsMouseClicked(1))
            ImGui::OpenPopup("HierarchyOptions");
    }

    if (ImGui::BeginPopup("HierarchyOptions"))
    {
        if (ImGui::BeginMenu("create"))
        {
            if (ImGui::MenuItem((std::format("{} {}", icon::entity, "GameObject")).c_str()))
                EntityManager::CreateNode("Gamebject", m_currentSelection);

            EntityManager::IterateRegisteredComponents([entity = m_currentSelection](const FComponentMeta& meta) {
                    if (!EntityManager::HasComponent(entity, meta.type)) {
                        if (ImGui::MenuItem(meta.name.data())) {
                            EntityManager::AddComponent(entity, meta.type);
                        }
                    }
                });

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("copy", "CTRL+C"))
            m_copySelection = m_currentSelection;

        if (ImGui::MenuItem("paste", "CTRL+V", nullptr, m_copySelection != entt::null))
        {
            //actionBuffer->addOperation(std::make_unique<CCopyEntityOperation>(m_currentSelection, m_copySelection));
            m_copySelection = entt::null;
        }

        if (ImGui::MenuItem("duplicate"))
        {
            entt::entity attach_to{ root };
            auto& hierarchy = EntityManager::GetComponent<ecs::FHierarchyComponent>(m_currentSelection);

            if (hierarchy.parent != entt::null)
                attach_to = hierarchy.parent;

            //actionBuffer->addOperation(std::make_unique<CDuplicateEntityOperation>(attach_to, m_currentSelection));
        }

        if (ImGui::MenuItem("delete", "DEL"))
        {
            auto& state = EditorState::GetInstance();
            if (state->IsSelected(m_currentSelection))
                state->SetSelection(root);
            
            //state->AddOperation(std::make_unique<CRemoveEntityOperation>(m_currentSelection));
        }

        ImGui::EndPopup();
    }

    float size_x = static_cast<float>(current_size.x);
    float size_y = static_cast<float>(current_size.y) - ImGui::GetCursorPosY() - 10.f;
    ImGui::SetCursorPosX(0);
    if (ImGui::InvisibleButton("###unselect_all_btn", ImVec2(size_x, size_y)))
    {
        m_currentSelection = root;
        EditorState::GetInstance()->DeselectAll();
    }
}

void HierarchyWindow::BuildHierarchy(const entt::entity& entity)
{
    auto& state = EditorState::GetInstance();

    if (entity != entt::null)
    {
        std::string icon{};

        uint32_t flags = ImGuiTreeNodeFlags_OpenOnArrow;

        auto& base = EntityManager::GetComponent<ecs::FBaseComponent>(entity);
        auto& hierarchy = EntityManager::GetComponent<ecs::FHierarchyComponent>(entity);

        //Is object selected
        auto isSelected = state->IsSelected(entity);
        if (isSelected)
            flags |= ImGuiTreeNodeFlags_Selected;

        //Has object childs
        if (hierarchy.children.empty())
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        else
            icon = ICON_MDI_GRAPH;

        ImGui::PushID(static_cast<uint32_t>(entity));
        bool isOpen = ImGui::TreeNodeEx((icon + " " + base.name).c_str(), flags);
        ImGui::PopID();

        if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
            m_currentSelection = entity;

        //Mouse double click event
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            //TODO: Add object lookAt
        }

        if (ImGui::IsItemHovered())
        {
            // Ctrl + click
            if (ImGui::IsKeyPressed(ImGuiKey_LeftCtrl) && ImGui::IsItemClicked(0))
            {
                if (isSelected)
                    state->DeselectOne(entity);
                else
                    state->SetSelection(entity);
            }
            //Mouse click event
            else if (!ImGui::IsKeyPressed(ImGuiKey_LeftCtrl) && ImGui::IsItemClicked(0))
            {
                state->SetSelection(entity);
            }
        }

        if (isOpen && !hierarchy.children.empty())
        {
            for (auto& child : hierarchy.children)
                BuildHierarchy(child);

            ImGui::TreePop();
        }

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("scene_node", &entity, sizeof(entity));
            ImGui::Text("%s", base.name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            auto payload = ImGui::AcceptDragDropPayload("scene_node");
            if (payload)
            {
                auto node = *static_cast<entt::entity*>(payload->Data);
                //auto& actionBuffer = EGEditor->getActionBuffer();
                //actionBuffer->addOperation(std::make_unique<CExchangeEntityOperation>(node, entity));
            }

            ImGui::EndDragDropTarget();
        }
    }
}