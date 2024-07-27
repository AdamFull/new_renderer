#include "HierarchyWindow.h"

#include <imgui.h>
#include <engine/world/EntityManager.h>

#include <engine/world/ecs/components/components.h>

#include <engine/integrations/imgui/font/IconsMaterialDesignIcons.h>

using namespace engine;
using namespace engine::world;

template<typename _Ty>
struct IconByComponentType;

template<>
struct IconByComponentType<entt::entity>
{ static constexpr const char* value = ICON_MDI_GRAPH; };

template<>
struct IconByComponentType<ecs::FTransformComponent>
{ static constexpr const char* value = ICON_MDI_AXIS; /*ICON_MDI_VECTOR_LINE*/ };

template<>
struct IconByComponentType<ecs::FSpotLightComponent>
{ static constexpr const char* value = ICON_MDI_HEAD_LIGHTBULB; };

template<>
struct IconByComponentType<ecs::FPointLightComponent>
{ static constexpr const char* value = ICON_MDI_LIGHTBULB; };

template<>
struct IconByComponentType<ecs::FDirectionalLightComponent>
{ static constexpr const char* value = ICON_MDI_BRIGHTNESS_7; };

//template<>
//struct IconByComponentType<ecs::FAudioComponent>
//{ static constexpr const char* value = ICON_MDI_VOLUME_HIGH; };

template<>
struct IconByComponentType<ecs::FCameraComponent>
{ static constexpr const char* value = ICON_MDI_CAMERA; };

template<>
struct IconByComponentType<ecs::FMeshComponent>
{ static constexpr const char* value = ICON_MDI_SHAPE; };

//template<>
//struct IconByComponentType<ecs::FRigidBodyComponent>
//{ static constexpr const char* value = ICON_MDI_SHAPE; };

//template<>
//struct IconByComponentType<ecs::FScriptComponent>
//{ static constexpr const char* value = ICON_MDI_SCRIPT; };
//
//template<>
//struct IconByComponentType<ecs::FEnvironmentComponent>
//{ static constexpr const char* value = ICON_MDI_EARTH; };
//
//template<>
//struct IconByComponentType<ecs::FSpriteComponent>
//{ static constexpr const char* value = ICON_MDI_IMAGE; };
//
//template<>
//struct IconByComponentType<ecs::FParticleComponent>
//{ static constexpr const char* value = ICON_MDI_CREATION; };
//
//template<>
//struct IconByComponentType<ecs::FTerrainComponent>
//{ static constexpr const char* value = ICON_MDI_CREATION; };

template<typename _Ty>
constexpr const char* IconByComponentType_v = IconByComponentType<_Ty>::value;


template<class _Ty>
void try_add_menu_item_h(const std::string& name, const entt::entity& entity)
{
    using namespace engine;
    //auto& actionBuffer = EGEditor->getActionBuffer();
    if (!CEntityManager::TryGetComponent<_Ty>(entity))
    {
        const char* icon = IconByComponentType_v<_Ty>;
        if (ImGui::MenuItem((std::format("{} {}", icon, name)).c_str()));
            //actionBuffer->addOperation(std::make_unique<CCreateEntityOperation>(entity, utl::type_hash<_Ty>()));
    }
}

void CEditorHierarchy::__draw(float fDt)
{
    //auto& actionBuffer = EGEditor->getActionBuffer();
    auto root = CEntityManager::GetRoot();
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
            try_add_menu_item_h<entt::entity>("entity", selected_entity);
            //try_add_menu_item_h<ecs::FAudioComponent>("audio", selected_entity);
            try_add_menu_item_h<ecs::FCameraComponent>("camera", selected_entity);
            //try_add_menu_item_h<ecs::FEnvironmentComponent>("environment", selected_entity);
            try_add_menu_item_h<ecs::FDirectionalLightComponent>("directional light", selected_entity);
            try_add_menu_item_h<ecs::FSpotLightComponent>("spot light", selected_entity);
            try_add_menu_item_h<ecs::FPointLightComponent>("point light", selected_entity);
            //try_add_menu_item_h<ecs::FSceneComponent>("scene", selected_entity);
            //try_add_menu_item_h<ecs::FRigidBodyComponent>("rigidbody", selected_entity);
            //try_add_menu_item_h<ecs::FParticleComponent>("particle", selected_entity);
            //try_add_menu_item_h<ecs::FTerrainComponent>("terrain", selected_entity);
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("copy", "CTRL+C"))
            copy_entity = selected_entity;

        if (ImGui::MenuItem("paste", "CTRL+V", nullptr, copy_entity != entt::null))
        {
            //actionBuffer->addOperation(std::make_unique<CCopyEntityOperation>(selected_entity, copy_entity));
            copy_entity = entt::null;
        }

        if (ImGui::MenuItem("duplicate"))
        {
            entt::entity attach_to{ root };
            auto& hierarchy = CEntityManager::GetComponent<ecs::FHierarchyComponent>(selected_entity);

            if (hierarchy.parent != entt::null)
                attach_to = hierarchy.parent;

            //actionBuffer->addOperation(std::make_unique<CDuplicateEntityOperation>(attach_to, selected_entity));
        }

        if (ImGui::MenuItem("delete", "DEL"))
        {
            //if (EGEditor->isSelected(selected_entity))
            //    EGEditor->selectObject(root);
            //
            //actionBuffer->addOperation(std::make_unique<CRemoveEntityOperation>(selected_entity));
        }

        ImGui::EndPopup();
    }

    float size_x = static_cast<float>(current_size.x);
    float size_y = static_cast<float>(current_size.y) - ImGui::GetCursorPosY() - 10.f;
    ImGui::SetCursorPosX(0);
    if (ImGui::InvisibleButton("###unselect_all_btn", ImVec2(size_x, size_y)))
    {
        selected_entity = root;
        //EGEditor->deselectAll();
    }
}

void CEditorHierarchy::BuildHierarchy(const entt::entity& entity)
{
    if (entity != entt::null)
    {
        std::string icon{};

        uint32_t flags = ImGuiTreeNodeFlags_OpenOnArrow;

        auto& base = CEntityManager::GetComponent<ecs::FBaseComponent>(entity);
        auto& hierarchy = CEntityManager::GetComponent<ecs::FHierarchyComponent>(entity);

        //Is object selected
        auto isSelected = false;//EGEditor->isSelected(entity);
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
            selected_entity = entity;

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
                if (isSelected);
                //EGEditor->deselectObject();
                else;
                    //EGEditor->selectObject(entity);
            }
            //Mouse click event
            else if (!ImGui::IsKeyPressed(ImGuiKey_LeftCtrl) && ImGui::IsItemClicked(0))
            {
                //EGEditor->deselectAll();
                //EGEditor->selectObject(entity);
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
            ImGui::Text(base.name.c_str());
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