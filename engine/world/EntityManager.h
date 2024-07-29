#pragma once

#include <functional>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <standart_library.h>

#include <engine/world/ecs/systems/BaseSystem.h>
#include <engine/world/ecs/components/components.h>

namespace engine
{
	namespace world
	{
		class CEntityManager
		{
		public:
			using serialize_function_t = std::function<void(entt::registry&, entt::entity, nlohmann::json&)>;
			using deserialize_function_t = std::function<void(entt::registry&, entt::entity, const nlohmann::json&)>;

			template<typename _Ty>
			static void RegisterComponent()
			{
				entt::id_type type = entt::type_hash<_Ty>::value();
			}

			template<typename _Ty, typename... _Args>
			static void RegisterSystem(_Args&&... args)
			{
				m_vSystems.emplace_back(std::make_unique<_Ty>(std::forward<_Args>(args)...));
			}

			static entt::entity CreateRoot(const std::string& name);
			static entt::entity CreateNode(const std::string& name = "", entt::entity parent = entt::null);
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
						func(static_cast<ecs::IComponent*>(set.get(entity)));
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

			static entt::registry& GetRegistry() { return m_registry; }
			static entt::entity GetRoot() { return m_root; }

			static nlohmann::json SerializeEntity(entt::entity entity);
			static entt::entity DeserializeEntity(entt::entity parent, const nlohmann::json& j);

			static void Create();
			static void Update(f32 dt);

		private:
			static entt::registry m_registry;
			static entt::entity m_root;
			static utl::vector<std::unique_ptr<ecs::ISystem>> m_vSystems;
		};
	}
}