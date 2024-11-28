#include "EntityManager.h"

#include <util/Logger.h>

#include <tracy/Tracy.hpp>

using namespace engine::world;

entt::registry EntityManager::m_registry{};
entt::entity EntityManager::m_root{};
utl::vector<std::unique_ptr<ecs::ISystem>> EntityManager::m_vSystems;
utl::vector<FComponentMeta> EntityManager::m_meta;

entt::entity EntityManager::CreateRoot(const std::string& name)
{
	if (m_registry.valid(m_root))
		return m_root;

	m_root = CreateNode(name);
	m_registry.emplace<ecs::FRootComponent>(m_root, ecs::FRootComponent{});
	return m_root;
}

entt::entity EntityManager::CreateNode(const std::string& name, entt::entity parent)
{
	entt::entity entity = m_registry.create();
	m_registry.emplace<ecs::FBaseComponent>(entity, ecs::FBaseComponent{ name });
	m_registry.emplace<ecs::FTransformComponent>(entity, ecs::FTransformComponent{});
	m_registry.emplace<ecs::FHierarchyComponent>(entity, ecs::FHierarchyComponent{ parent });

	if(parent != entt::null)
		AttachChild(parent, entity);
	
	log_debug("[Scene Graph] Entity with id {} was created", static_cast<uint32_t>(entity));

	return entity;
}

void EntityManager::DestroyNode(entt::entity node)
{
	DetachChild(node);
	m_registry.destroy(node);
	log_debug("[Scene Graph] Entity with id {} was destroyed", static_cast<uint32_t>(node));
}

void EntityManager::AttachChild(entt::entity parent, entt::entity child)
{
	if (parent == child)
	{
		log_warning("[Scene Graph] Trying to add parent to parent. Parent entity {}, child entity {}.", static_cast<uint32_t>(parent), static_cast<uint32_t>(child));
		return;
	}

	auto& phierarchy = m_registry.get<ecs::FHierarchyComponent>(parent);
	phierarchy.children.emplace_back(child);

	auto& chierarchy = m_registry.get<ecs::FHierarchyComponent>(child);
	chierarchy.parent = parent;
}

void EntityManager::DetachChild(entt::entity parent, entt::entity child)
{
	if (parent == entt::null)
		return;

	auto& phierarchy = m_registry.get<ecs::FHierarchyComponent>(parent);
	auto cit = std::remove(phierarchy.children.begin(), phierarchy.children.end(), child);

	if (cit != phierarchy.children.end())
		phierarchy.children.erase(cit);
	else
		log_error("[Scene Graph] Entity with is {} is not child of {}", static_cast<uint32_t>(child), static_cast<uint32_t>(parent));

	auto& chierarchy = m_registry.get<ecs::FHierarchyComponent>(child);
	chierarchy.parent = entt::null;
}

void EntityManager::DetachChild(entt::entity child)
{
	auto& chierarchy = m_registry.get<ecs::FHierarchyComponent>(child);
	DetachChild(chierarchy.parent, child);
}

entt::entity EntityManager::Instantiate(entt::entity prefab)
{
	return entt::null;
}

bool EntityManager::IsValid(entt::entity entity)
{
	return m_registry.valid(entity);
}

void* EntityManager::AddComponent(entt::entity entity, entt::id_type type)
{
	if (auto found = std::find_if(m_meta.begin(), m_meta.end(), [type](const FComponentMeta& meta) {return meta.type == type; }); found != m_meta.end())
		return found->create(entity);

	return nullptr;
}

void* EntityManager::TryGetComponent(entt::entity entity, entt::id_type type)
{
	if (auto found = std::find_if(m_meta.begin(), m_meta.end(), [type](const FComponentMeta& meta) {return meta.type == type; }); found != m_meta.end())
		return found->try_get(entity);

	return nullptr;
}

bool EntityManager::HasComponent(entt::entity entity, entt::id_type type)
{
	if (auto found = std::find_if(m_meta.begin(), m_meta.end(), [type](const FComponentMeta& meta) {return meta.type == type; }); found != m_meta.end())
		return found->has(entity);

	return false;
}

nlohmann::json EntityManager::SerializeEntity(entt::entity entity)
{
	nlohmann::json j = nlohmann::json::object();

	auto& base = m_registry.get<ecs::FBaseComponent>(entity);
	j["name"] = base.name;
	j["enabled"] = base.enabled;
	j["visible"] = base.visible;

	auto& components = j["components"];
	IterateComponents(entity, [&components, entity](const FComponentMeta& meta) { meta.serialize(components, entity); });

	auto& children = j["children"];
	auto& hierarchy = m_registry.get<ecs::FHierarchyComponent>(entity);
	for (entt::entity child : hierarchy.children)
		children.emplace_back(SerializeEntity(child));

	return j;
}

entt::entity EntityManager::DeserializeEntity(entt::entity parent, const nlohmann::json& j)
{
	entt::entity entity = CreateNode("", parent);

	auto& base = m_registry.get<ecs::FBaseComponent>(entity);
	j.at("name").get_to(base.name);
	j.at("enabled").get_to(base.enabled);
	j.at("visible").get_to(base.visible);

	auto& components = j["components"];
	IterateComponents(entity, [&components, entity](const FComponentMeta& meta) { meta.deserialize(components, entity); });

	auto& hierarchy = m_registry.get<ecs::FHierarchyComponent>(entity);
	auto& children = j["children"];
	for (const auto& child : children)
		hierarchy.children.emplace_back(DeserializeEntity(entity, child));

	return entity;
}

void EntityManager::Create()
{
	for (auto& system : m_vSystems)
		system->Create();
}

void EntityManager::Update(f32 dt)
{
	ZoneScoped;
	for (auto& system : m_vSystems)
		system->Update(dt);
}