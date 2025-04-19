#include "EntityManager.h"

#include <edge/util/Logger.h>

#include <edge/util/utl_profiler.h>

using namespace engine::world;

entt::registry EntityManager::m_registry{};
entt::entity EntityManager::m_root{};
std::vector<std::unique_ptr<ecs::ISystem>> EntityManager::m_vSystems;
std::vector<ComponentMeta> EntityManager::m_meta;

auto EntityManager::create_root(std::string_view name) -> entt::entity {
	if (m_registry.valid(m_root)) {
		return m_root;
	}

	m_root = create_node(name);
	m_registry.emplace<ecs::RootComponent>(m_root, ecs::RootComponent{});
	return m_root;
}

auto EntityManager::create_node(std::string_view name, entt::entity parent) -> entt::entity {
	entt::entity entity = m_registry.create();
	m_registry.emplace<ecs::BaseComponent>(entity, ecs::BaseComponent{ name });
	m_registry.emplace<ecs::TransformComponent>(entity, ecs::TransformComponent{});
	m_registry.emplace<ecs::HierarchyComponent>(entity, ecs::HierarchyComponent{ parent });

	if (parent != entt::null) {
		attach_child(parent, entity);
	}
	
	EDGE_LOGT("[Scene Graph] Entity with id {} was created", static_cast<uint32_t>(entity));

	return entity;
}

void EntityManager::destroy_node(entt::entity node) {
	detach_child(node);
	m_registry.destroy(node);
	EDGE_LOGT("[Scene Graph] Entity with id {} was destroyed", static_cast<uint32_t>(node));
}

void EntityManager::attach_child(entt::entity parent, entt::entity child) {
	if (parent == child) {
		EDGE_LOGW("[Scene Graph] Trying to add parent to parent. Parent entity {}, child entity {}.", static_cast<uint32_t>(parent), static_cast<uint32_t>(child));
		return;
	}

	auto& phierarchy = m_registry.get<ecs::HierarchyComponent>(parent);
	phierarchy.children.emplace_back(child);

	auto& chierarchy = m_registry.get<ecs::HierarchyComponent>(child);
	chierarchy.parent = parent;
}

void EntityManager::detach_child(entt::entity parent, entt::entity child) {
	if (parent == entt::null) {
		return;
	}

	auto& phierarchy = m_registry.get<ecs::HierarchyComponent>(parent);
	auto cit = std::remove(phierarchy.children.begin(), phierarchy.children.end(), child);

	if (cit != phierarchy.children.end()) {
		phierarchy.children.erase(cit);
	}
	else {
		EDGE_LOGE("[Scene Graph] Entity with is {} is not child of {}", static_cast<uint32_t>(child), static_cast<uint32_t>(parent));
	}

	auto& chierarchy = m_registry.get<ecs::HierarchyComponent>(child);
	chierarchy.parent = entt::null;
}

void EntityManager::detach_child(entt::entity child) {
	auto& chierarchy = m_registry.get<ecs::HierarchyComponent>(child);
	detach_child(chierarchy.parent, child);
}

auto EntityManager::instantiate(entt::entity prefab) -> entt::entity {
	// TODO: implement for create instances from prefabs
	return entt::null;
}

auto EntityManager::is_valid(entt::entity entity) -> bool {
	return m_registry.valid(entity);
}

auto EntityManager::add_component(entt::entity entity, entt::id_type type) -> void* {
	if (auto found = std::find_if(m_meta.begin(), m_meta.end(),
		[type](const ComponentMeta& meta) {
			return meta.type == type;
		}); found != m_meta.end()) {
		return found->create(entity);
	}

	return nullptr;
}

void* EntityManager::try_get_component(entt::entity entity, entt::id_type type) {
	if (auto found = std::find_if(m_meta.begin(), m_meta.end(),
		[type](const ComponentMeta& meta) {
			return meta.type == type;
		}); found != m_meta.end()) {
		return found->try_get(entity);
	}

	return nullptr;
}

bool EntityManager::has_component(entt::entity entity, entt::id_type type) {
	if (auto found = std::find_if(m_meta.begin(), m_meta.end(),
		[type](const ComponentMeta& meta) {
			return meta.type == type;
		}); found != m_meta.end()) {
		return found->has(entity);
	}

	return false;
}

nlohmann::json EntityManager::serialize_entity(entt::entity entity) {
	nlohmann::json j = nlohmann::json::object();

	auto& base = m_registry.get<ecs::BaseComponent>(entity);
	j["name"] = base.name;
	j["enabled"] = base.enabled;
	j["visible"] = base.visible;

	auto& components = j["components"];
	iterate_components(entity,
		[&components, entity](const ComponentMeta& meta) { 
			meta.serialize(components, entity); 
		});

	auto& children = j["children"];
	auto& hierarchy = m_registry.get<ecs::HierarchyComponent>(entity);
	for (entt::entity child : hierarchy.children) {
		children.emplace_back(serialize_entity(child));
	}

	return j;
}

entt::entity EntityManager::deserialize_entity(entt::entity parent, const nlohmann::json& j) {
	entt::entity entity = create_node("", parent);

	auto& base = m_registry.get<ecs::BaseComponent>(entity);
	j.at("name").get_to(base.name);
	j.at("enabled").get_to(base.enabled);
	j.at("visible").get_to(base.visible);

	auto& components = j["components"];
	iterate_components(entity,
		[&components, entity](const ComponentMeta& meta) { 
			meta.deserialize(components, entity); 
		});

	auto& hierarchy = m_registry.get<ecs::HierarchyComponent>(entity);
	auto& children = j["children"];
	for (const auto& child : children)
		hierarchy.children.emplace_back(deserialize_entity(entity, child));

	return entity;
}

void EntityManager::create() {
	for (auto& system : m_vSystems) {
		system->create();
	}
}

void EntityManager::update(float dt) {
	EDGE_PROFILE_ZONE_SCOPED;
	for (auto& system : m_vSystems) {
		system->update(dt);
	}
}