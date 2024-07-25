#include "EntityComponentManager.h"

#include <logger/logger.h>

using namespace engine::world;

entt::registry CEntityComponentManager::m_registry{};
entt::entity CEntityComponentManager::m_root{};
utl::vector<std::unique_ptr<ecs::ISystem>> CEntityComponentManager::m_vSystems;
utl::hash_map<entt::id_type, CEntityComponentManager::serialize_function_t> CEntityComponentManager::m_componentSerializers{};
utl::hash_map<entt::id_type, CEntityComponentManager::deserialize_function_t> CEntityComponentManager::m_componentDeserializers{};

entt::entity CEntityComponentManager::CreateRoot(const std::string& name)
{
	if (m_registry.valid(m_root))
		return m_root;

	m_root = CreateNode(name);
	m_registry.emplace<ecs::FRootComponent>(m_root, ecs::FRootComponent{});
	return m_root;
}

entt::entity CEntityComponentManager::CreateNode(const std::string& name, entt::entity parent)
{
	entt::entity entity = m_registry.create();
	m_registry.emplace<ecs::FBaseComponent>(entity, ecs::FBaseComponent{ name });
	m_registry.emplace<ecs::FTransformComponent>(entity, ecs::FTransformComponent{});
	m_registry.emplace<ecs::FHierarchyComponent>(entity, ecs::FHierarchyComponent{ parent });

	log_debug("[Scene Graph] Entity with id {} was created", static_cast<uint32_t>(entity));

	return entity;
}

void CEntityComponentManager::DestroyNode(entt::entity node)
{
	DetachChild(node);
	m_registry.destroy(node);
	log_debug("[Scene Graph] Entity with id {} was destroyed", static_cast<uint32_t>(node));
}

void CEntityComponentManager::AttachChild(entt::entity parent, entt::entity child)
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

void CEntityComponentManager::DetachChild(entt::entity parent, entt::entity child)
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

void CEntityComponentManager::DetachChild(entt::entity child)
{
	auto& chierarchy = m_registry.get<ecs::FHierarchyComponent>(child);
	DetachChild(chierarchy.parent, child);
}

entt::entity Instantiate(entt::entity prefab)
{
	return entt::null;
}

nlohmann::json CEntityComponentManager::SerializeEntity(entt::entity entity)
{
	nlohmann::json j = nlohmann::json::object();

	auto& base = m_registry.get<ecs::FBaseComponent>(entity);
	j["name"] = base.name;
	j["enabled"] = base.enabled;
	j["visible"] = base.visible;

	auto& components = j["components"];
	for (const auto& [type, serializer] : m_componentSerializers)
		serializer(m_registry, entity, components);

	auto& children = j["children"];
	auto& hierarchy = m_registry.get<ecs::FHierarchyComponent>(entity);
	for (entt::entity child : hierarchy.children)
		children.push_back(SerializeEntity(child));

	return j;
}

entt::entity CEntityComponentManager::DeserializeEntity(entt::entity parent, const nlohmann::json& j)
{
	entt::entity entity = CreateNode("", parent);

	auto& base = m_registry.get<ecs::FBaseComponent>(entity);
	j.at("name").get_to(base.name);
	j.at("enabled").get_to(base.enabled);
	j.at("visible").get_to(base.visible);

	auto& components = j["components"];
	for (const auto& [type, deserializer] : m_componentDeserializers)
		deserializer(m_registry, entity, components);

	auto& hierarchy = m_registry.get<ecs::FHierarchyComponent>(entity);
	auto& children = j["children"];
	for (const auto& child : children)
		hierarchy.children.emplace_back(DeserializeEntity(entity, child));

	return entity;
}