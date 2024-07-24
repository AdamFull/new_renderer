#include "SceneGraph.h"

#include <logger/logger.h>
#include <engine/world/ecs/components/components.h>

#include "GameObject.h"

using namespace engine::world;
using namespace engine::world::ecs;

CSceneGraph::CSceneGraph()
{
	m_pRegistry = std::make_unique<entt::registry>();
}

entt::entity CSceneGraph::CreateRoot(const std::string& name)
{
	if (m_pRegistry->valid(m_root))
		return m_root;

	m_root = CreateNode(name);
	m_pRegistry->emplace<FRootComponent>(m_root, FRootComponent{});
	return m_root;
}

entt::entity CSceneGraph::CreateNode(const std::string& name)
{
	auto entity = m_pRegistry->create();
	m_pRegistry->emplace<FBaseComponent>(entity, FBaseComponent{name});
	m_pRegistry->emplace<FTransformComponent>(entity, FTransformComponent{});
	m_pRegistry->emplace<FHierarchyComponent>(entity, FHierarchyComponent{});
	log_debug("[Scene Graph] Entity with id {} was created", static_cast<uint32_t>(entity));
	return entity;
}

void CSceneGraph::DestroyNode(entt::entity node)
{
	DetachChild(node);
	m_pRegistry->destroy(node);
	log_debug("[Scene Graph] Entity with id {} was destroyed", static_cast<uint32_t>(node));
}

void CSceneGraph::AttachChild(entt::entity parent, entt::entity child)
{
	if (parent == child)
	{
		log_warning("[Scene Graph] Trying to add parent to parent. Parent entity {}, child entity {}.", static_cast<uint32_t>(parent), static_cast<uint32_t>(child));
		return;
	}

	auto& phierarchy = m_pRegistry->get<FHierarchyComponent>(parent);
	phierarchy.children.emplace_back(child);

	auto& chierarchy = m_pRegistry->get<FHierarchyComponent>(child);
	chierarchy.parent = parent;
}

void CSceneGraph::DetachChild(entt::entity parent, entt::entity child)
{
	if (parent == entt::null)
		return;

	auto& phierarchy = m_pRegistry->get<FHierarchyComponent>(parent);
	auto cit = std::remove(phierarchy.children.begin(), phierarchy.children.end(), child);

	if (cit != phierarchy.children.end())
		phierarchy.children.erase(cit);
	else
		log_error("[Scene Graph] Entity with is {} is not child of {}", static_cast<uint32_t>(child), static_cast<uint32_t>(parent));

	auto& chierarchy = m_pRegistry->get<FHierarchyComponent>(child);
	chierarchy.parent = entt::null;
}

void CSceneGraph::DetachChild(entt::entity child)
{
	auto& chierarchy = m_pRegistry->get<FHierarchyComponent>(child);
	DetachChild(chierarchy.parent, child);
}

entt::entity CSceneGraph::duplicateNode(entt::entity entity)
{
	/*auto& transform = registry.get<FTransformComponent>(entity);
	auto& hierarchy = registry.get<FHierarchyComponent>(entity);

	auto new_node = create_node(hierarchy.name);
	registry.replace<FTransformComponent>(new_node, transform);

	if (auto audio = registry.try_get<FAudioComponent>(entity))
		registry.emplace<FAudioComponent>(new_node, *audio);

	// Skybox can be only one!

	if (auto audio = registry.try_get<FAudioComponent>(entity))
		registry.emplace<FAudioComponent>(new_node, *audio);

	if (auto mesh = registry.try_get<FMeshComponent>(entity))
		registry.emplace<FMeshComponent>(new_node, *mesh);

	if (auto sprite = registry.try_get<FSpriteComponent>(entity))
		registry.emplace<FSpriteComponent>(new_node, *sprite);

	if (auto camera = registry.try_get<FCameraComponent>(entity))
	{
		registry.emplace<FCameraComponent>(new_node, *camera);
		auto& ncamera = registry.get<FCameraComponent>(new_node);
		ncamera.active = false;
	}

	if (auto script = registry.try_get<FScriptComponent>(entity))
		registry.emplace<FScriptComponent>(new_node, *script);

	if (auto light = registry.try_get<FDirectionalLightComponent>(entity))
		registry.emplace<FDirectionalLightComponent>(new_node, *light);

	if (auto light = registry.try_get<FPointLightComponent>(entity))
		registry.emplace<FPointLightComponent>(new_node, *light);

	if (auto light = registry.try_get<FSpotLightComponent>(entity))
		registry.emplace<FSpotLightComponent>(new_node, *light);

	for (auto& child : hierarchy.children)
		attach_child(new_node, duplicate_node(child));

	return new_node;*/
	return entt::entity();
}

void CSceneGraph::parentExchange(entt::entity new_parent, entt::entity child)
{
	auto& chierarchy = m_pRegistry->get<FHierarchyComponent>(child);
	DetachChild(chierarchy.parent, child);
	AttachChild(new_parent, child);
}

const std::unique_ptr<entt::registry>& CSceneGraph::GetRegistry() const
{
	return m_pRegistry;
}

entt::entity CSceneGraph::GetRoot() const
{
	return m_root;
}

nlohmann::json CSceneGraph::SerializeGameObject(class CGameObject* go)
{
	nlohmann::json j;

	auto entity = go->GetEntity();

	return j;
}

void CSceneGraph::DeserializeGameObject(const nlohmann::json& j)
{

}