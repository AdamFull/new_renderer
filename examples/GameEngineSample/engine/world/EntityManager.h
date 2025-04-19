#pragma once

#include <functional>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "../ui/InspectorDrawer.h"

#include "ecs/systems/BaseSystem.h"
#include "ecs/components/components.h"

namespace engine::world {
	enum class EComponentFlagBits {
		eNone = 0,
		eHiddenInEditor = 1 << 0
	};
	EDGE_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(EComponentFlags, EComponentFlagBits);

	struct ComponentMeta {
		std::string_view name{};
		entt::id_type type{};
		EComponentFlags flags{};

		std::function<void* (entt::entity)> create;
		std::function<void* (entt::entity)> try_get;
		std::function<void(entt::entity)> destroy;
		std::function<bool(entt::entity)> has;
		std::function<void(InspectorDrawer&, entt::entity)> edit;
		std::function<void(nlohmann::json&, entt::entity)> serialize;
		std::function<void(const nlohmann::json&, entt::entity)> deserialize;
	};

	class EntityManager {
	public:
		template<typename _Ty>
		static void register_component(EComponentFlags flags = EComponentFlagBits::eNone) {
			constexpr auto type_desc = refl::reflect<_Ty>();

			auto& meta = m_meta.emplace_back();

			if constexpr (refl::descriptor::has_attribute<attr::Name>(type_desc)) {
				constexpr auto name_attr = refl::descriptor::get_attribute<attr::Name>(type_desc);
				meta.name = name_attr.name;
			}

			meta.type = entt::type_hash<_Ty>::value();
			meta.flags = flags;

			meta.create = [&registry = m_registry](entt::entity entity) -> void* { return (&registry.emplace<_Ty>(entity, _Ty{})); };
			meta.try_get = [&registry = m_registry](entt::entity entity) -> void* { return registry.try_get<_Ty>(entity); };
			meta.destroy = [&registry = m_registry](entt::entity entity) { registry.remove<_Ty>(entity); };
			meta.has = [&registry = m_registry](entt::entity entity) { return registry.any_of<_Ty>(entity); };

			meta.edit = [&registry = m_registry](InspectorDrawer& drawer, entt::entity entity) {
					if constexpr (refl::is_reflectable<_Ty>()) {
						auto* component = registry.try_get<_Ty>(entity);
						if (component) {
							drawer.draw_big(*component);
						}
					}
				};

			meta.serialize = [&registry = m_registry](nlohmann::json& j, entt::entity entity) {
					if constexpr (refl::is_reflectable<_Ty>()) {
						auto* component = registry.try_get<_Ty>(entity);
						if (component) {
							Serialize(j, *component);
						}
					}
				};

			meta.deserialize = [&registry = m_registry](const nlohmann::json& j, entt::entity entity) {
					if constexpr (refl::is_reflectable<_Ty>()) {
						auto* component = registry.try_get<_Ty>(entity);
						if (component) {
							Deserialize(j, *component);
						}
					}
				};
		}

		template<typename _Ty, typename... _Args>
		static void register_system(_Args&&... args) {
			m_vSystems.emplace_back(std::make_unique<_Ty>(std::forward<_Args>(args)...));
		}

		static auto create_root(std::string_view name) -> entt::entity;
		static auto create_node(std::string_view name = "", entt::entity parent = entt::null) -> entt::entity;
		static void destroy_node(entt::entity node);
		static void attach_child(entt::entity parent, entt::entity child);
		static void detach_child(entt::entity parent, entt::entity child);
		static void detach_child(entt::entity child);
		static auto instantiate(entt::entity prefab) -> entt::entity;

		static auto is_valid(entt::entity entity) -> bool;

		template<typename... _Components>
		static auto view() {
			return m_registry.view<_Components...>();
		}

		template<typename... _Components, typename _Func>
		static void each(_Func func) {
			auto v = m_registry.view<_Components...>();
			v.each(func);
		}

		template<typename _Func>
		static void iterate_components(entt::entity entity, _Func&& func) {
			for (auto&& storage : m_registry.storage()) {
				if (auto& set = storage.second; set.contains(entity)) {
					if (auto found = std::find_if(m_meta.begin(), m_meta.end(),
						[type = storage.first](const ComponentMeta& meta) {
							return meta.type == type;
						}); found != m_meta.end()) {
						func(*found);
					}
				}
			}
		}

		template<typename _Ty>
		static auto add_component(entt::entity entity, _Ty&& component) -> _Ty& {
			return m_registry.emplace<_Ty>(entity, std::forward<_Ty>(component));
		}

		template<typename _Ty>
		static auto get_component(entt::entity entity) -> _Ty& {
			return m_registry.get<_Ty>(entity);
		}

		template<typename _Ty>
		static auto try_get_component(entt::entity entity) -> _Ty* {
			return m_registry.try_get<_Ty>(entity);
		}

		template<typename _Ty>
		static auto has_component(entt::entity entity) -> bool {
			return m_registry.any_of<_Ty>(entity);
		}

		// Some runtime stuff
		static auto add_component(entt::entity entity, entt::id_type type) -> void*;
		static auto try_get_component(entt::entity entity, entt::id_type type) -> void*;
		static auto has_component(entt::entity entity, entt::id_type type) -> bool;

		template<typename _Func>
		static void iterate_registered_components(_Func&& func) {
			for (const auto& meta : m_meta) {
				func(meta);
			}
		}


		static auto get_registry() -> entt::registry& {
			return m_registry; 
		}

		static auto get_root() -> entt::entity { 
			return m_root; 
		}

		static auto serialize_entity(entt::entity entity) -> nlohmann::json;
		static auto deserialize_entity(entt::entity parent, const nlohmann::json& j) -> entt::entity;

		static void create();
		static void update(float dt);

	private:
		static entt::registry m_registry;
		static entt::entity m_root;
		static std::vector<std::unique_ptr<ecs::ISystem>> m_vSystems;
		static std::vector<ComponentMeta> m_meta;
	};
}