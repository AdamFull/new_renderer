#include "SceneSerializer.h"

#include <logger/logger.h>

using namespace engine::world;

utl::hash_map<entt::id_type, CSceneSerializer::serialize_function_t> CSceneSerializer::m_componentSerializers{};
utl::hash_map<entt::id_type, CSceneSerializer::deserialize_function_t> CSceneSerializer::m_componentDeserializers{};

entt::entity CSceneSerializer::CreateNode(entt::registry& registry, entt::entity parent, const std::string& name)
{
	entt::entity entity = registry.create();
	registry.emplace<ecs::FBaseComponent>(entity, ecs::FBaseComponent{ name });
	registry.emplace<ecs::FTransformComponent>(entity, ecs::FTransformComponent{});
	registry.emplace<ecs::FHierarchyComponent>(entity, ecs::FHierarchyComponent{ parent });

	log_debug("[Scene Graph] Entity with id {} was created", static_cast<uint32_t>(entity));

	return entity;
}

void CSceneSerializer::DestroyNode(entt::registry& registry, entt::entity node)
{
	DetachChild(registry, node);
	registry.destroy(node);
	log_debug("[Scene Graph] Entity with id {} was destroyed", static_cast<uint32_t>(node));
}

void CSceneSerializer::AttachChild(entt::registry& registry, entt::entity parent, entt::entity child)
{
	if (parent == child)
	{
		log_warning("[Scene Graph] Trying to add parent to parent. Parent entity {}, child entity {}.", static_cast<uint32_t>(parent), static_cast<uint32_t>(child));
		return;
	}

	auto& phierarchy = registry.get<ecs::FHierarchyComponent>(parent);
	phierarchy.children.emplace_back(child);

	auto& chierarchy = registry.get<ecs::FHierarchyComponent>(child);
	chierarchy.parent = parent;
}

void CSceneSerializer::DetachChild(entt::registry& registry, entt::entity parent, entt::entity child)
{
	if (parent == entt::null)
		return;

	auto& phierarchy = registry.get<ecs::FHierarchyComponent>(parent);
	auto cit = std::remove(phierarchy.children.begin(), phierarchy.children.end(), child);

	if (cit != phierarchy.children.end())
		phierarchy.children.erase(cit);
	else
		log_error("[Scene Graph] Entity with is {} is not child of {}", static_cast<uint32_t>(child), static_cast<uint32_t>(parent));

	auto& chierarchy = registry.get<ecs::FHierarchyComponent>(child);
	chierarchy.parent = entt::null;
}

void CSceneSerializer::DetachChild(entt::registry& registry, entt::entity child)
{
	auto& chierarchy = registry.get<ecs::FHierarchyComponent>(child);
	DetachChild(registry, chierarchy.parent, child);
}

entt::entity Instantiate(entt::registry& registry, entt::entity prefab)
{
	return entt::null;
}

nlohmann::json CSceneSerializer::SerializeEntity(entt::registry& registry, entt::entity entity)
{
	nlohmann::json j = nlohmann::json::object();

	auto& base = registry.get<ecs::FBaseComponent>(entity);
	j["name"] = base.name;
	j["enabled"] = base.enabled;
	j["visible"] = base.visible;

	auto& components = j["components"];
	for (const auto& [type, serializer] : m_componentSerializers)
		serializer(registry, entity, components);

	auto& children = j["children"];
	auto& hierarchy = registry.get<ecs::FHierarchyComponent>(entity);
	for (entt::entity child : hierarchy.children)
		children.push_back(SerializeEntity(registry, child));

	return j;
}

entt::entity CSceneSerializer::DeserializeEntity(entt::registry& registry, entt::entity parent, const nlohmann::json& j)
{
	entt::entity entity = CreateNode(registry, parent);

	auto& base = registry.get<ecs::FBaseComponent>(entity);
	j.at("name").get_to(base.name);
	j.at("enabled").get_to(base.enabled);
	j.at("visible").get_to(base.visible);

	auto& components = j["components"];
	for (const auto& [type, deserializer] : m_componentDeserializers)
		deserializer(registry, entity, components);

	auto& hierarchy = registry.get<ecs::FHierarchyComponent>(entity);
	auto& children = j["children"];
	for (const auto& child : children)
		hierarchy.children.emplace_back(DeserializeEntity(registry, entity, child));

	return entity;
}