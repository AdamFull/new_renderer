#pragma once

#include <functional>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <standart_library.h>

#include "../ui/InspectorDrawer.h"

#include "ecs/systems/BaseSystem.h"
#include "ecs/components/components.h"

namespace engine
{
	namespace world
	{
		enum class EComponentFlagBits
		{
			eNone = 0,
			eHiddenInEditor = 1 << 0
		};
		EDGE_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(EComponentFlags, EComponentFlagBits);

		struct FComponentMeta
		{
			const char* name{ nullptr };
			entt::id_type type{};
			EComponentFlags flags{};

			std::function<void* (entt::entity)> create;
			std::function<void* (entt::entity)> try_get;
			std::function<void(entt::entity)> destroy;
			std::function<bool(entt::entity)> has;
			std::function<void(InspectorDrawer&, entt::entity)> edit;
			std::function<void(nlohmann::json& , entt::entity)> serialize;
			std::function<void(const nlohmann::json& , entt::entity)> deserialize;
		};

		class EntityManager
		{
		public:
			template<typename _Ty>
			static void RegisterComponent(EComponentFlags flags = EComponentFlagBits::eNone)
			{
				auto& meta = m_meta.emplace_back();
				meta.name = _Ty::_Name.data();
				meta.type = entt::type_hash<_Ty>::value();
				meta.flags = flags;

				meta.create = [&registry = m_registry](entt::entity entity) -> void* { return (&registry.emplace<_Ty>(entity, _Ty{})); };
				meta.try_get = [&registry = m_registry](entt::entity entity) -> void* { return registry.try_get<_Ty>(entity); };
				meta.destroy = [&registry = m_registry](entt::entity entity) { registry.remove<_Ty>(entity); };
				meta.has = [&registry = m_registry](entt::entity entity) { return registry.any_of<_Ty>(entity); };

				meta.edit = [&registry = m_registry](InspectorDrawer& drawer, entt::entity entity)
					{
						if constexpr (refl::is_reflectable<_Ty>())
						{
							auto* component = registry.try_get<_Ty>(entity);
							if (component)
								drawer.Draw(*component);
						}
					};

				meta.serialize = [&registry = m_registry](nlohmann::json& j, entt::entity entity)
					{
						if constexpr (refl::is_reflectable<_Ty>())
						{
							auto* component = registry.try_get<_Ty>(entity);
							if (component)
								Serialize(j, *component);
						}
					};

				meta.deserialize = [&registry = m_registry](const nlohmann::json& j, entt::entity entity)
					{
						if constexpr (refl::is_reflectable<_Ty>())
						{
							auto* component = registry.try_get<_Ty>(entity);
							if (component)
								Deserialize(j, *component);
						}
					};
			}

			template<typename _Ty, typename... _Args>
			static void RegisterSystem(_Args&&... args)
			{
				m_vSystems.emplace_back(std::make_unique<_Ty>(std::forward<_Args>(args)...));
			}

			static entt::entity CreateRoot(std::string_view name);
			static entt::entity CreateNode(std::string_view name = "", entt::entity parent = entt::null);
			static void DestroyNode(entt::entity node);
			static void AttachChild(entt::entity parent, entt::entity child);
			static void DetachChild(entt::entity parent, entt::entity child);
			static void DetachChild(entt::entity child);
			static entt::entity Instantiate(entt::entity prefab);

			static bool IsValid(entt::entity entity);

			template<typename... _Components>
			static auto View() 
			{
				return m_registry.view<_Components...>();
			}

			template<typename... _Components, typename _Func>
			static void Each(_Func func) 
			{
				auto v = m_registry.view<_Components...>();
				v.each(func);
			}

			template<typename _Func>
			static void IterateComponents(entt::entity entity, _Func&& func)
			{
				for (auto&& storage : m_registry.storage())
				{
					if (auto& set = storage.second; set.contains(entity))
					{
						if (auto found = std::find_if(m_meta.begin(), m_meta.end(), [type = storage.first](const FComponentMeta& meta) {return meta.type == type; }); found != m_meta.end())
							func(*found);
					}
				}
			}

			template<typename _Ty>
			static _Ty& AddComponent(entt::entity entity, _Ty&& component)
			{
				return m_registry.emplace<_Ty>(entity, std::forward<_Ty>(component));
			}

			template<typename _Ty>
			static _Ty& GetComponent(entt::entity entity)
			{
				return m_registry.get<_Ty>(entity);
			}

			template<typename _Ty>
			static _Ty* TryGetComponent(entt::entity entity)
			{
				return m_registry.try_get<_Ty>(entity);
			}

			template<typename _Ty>
			static bool HasComponent(entt::entity entity)
			{
				return m_registry.any_of<_Ty>(entity);
			}

			// Some runtime stuff
			static void* AddComponent(entt::entity entity, entt::id_type type);
			static void* TryGetComponent(entt::entity entity, entt::id_type type);
			static bool HasComponent(entt::entity entity, entt::id_type type);

			template<typename _Func>
			static void IterateRegisteredComponents(_Func&& func)
			{
				for (const auto& meta : m_meta)
					func(meta);
			}


			static entt::registry& GetRegistry() { return m_registry; }
			static entt::entity GetRoot() { return m_root; }

			static nlohmann::json SerializeEntity(entt::entity entity);
			static entt::entity DeserializeEntity(entt::entity parent, const nlohmann::json& j);

			static void Create();
			static void Update(f32 dt);

		private:
			static entt::registry m_registry;
			static entt::entity m_root;
			static std::vector<std::unique_ptr<ecs::ISystem>> m_vSystems;
			static std::vector<FComponentMeta> m_meta;
		};
	}
}