#include "GameObject.h"

#include <typeindex>
#include <entt/entt.hpp>

#include "ecs/components/TransformComponent.h"
#include "ecs/components/HierarchyComponent.h"

using namespace engine::world;

void CGameObject::AddChild(CGameObject* child)
{
    auto& node = m_pRegistry->get<ecs::FHierarchyComponent>(m_self);
    node.children.emplace_back(child->m_self);
    m_pRegistry->get<ecs::FHierarchyComponent>(child->m_self).parent = m_self;
}

void CGameObject::RemoveChild(CGameObject* child)
{
    auto& node = m_pRegistry->get<ecs::FHierarchyComponent>(m_self);
    auto to_remove = std::remove(node.children.begin(), node.children.end(), child->m_self);
    node.children.erase(to_remove, node.children.end());
    m_pRegistry->get<ecs::FHierarchyComponent>(child->m_self).parent = entt::null;
}

CGameObject* CGameObject::GetParent()
{
    return nullptr;
}