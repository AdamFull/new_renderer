#pragma once

#include <nlohmann/json.hpp>
#include <entt/entt.hpp>
#include <standart_library.h>
#include <functional>

namespace engine
{
	namespace world
	{
		class CSceneGraph
		{
		public:
			CSceneGraph();

			entt::entity CreateRoot(const std::string& name);
			entt::entity CreateNode(const std::string& name);
			void DestroyNode(entt::entity node);
			void AttachChild(entt::entity parent, entt::entity child);
			void DetachChild(entt::entity parent, entt::entity child);
			void DetachChild(entt::entity child);
			entt::entity duplicateNode(entt::entity child);
			void parentExchange(entt::entity new_parent, entt::entity child);

			const std::unique_ptr<entt::registry>& GetRegistry() const;
			entt::entity GetRoot() const;

			template<class _Ty>
			_Ty& AddComponent(entt::entity entity, _Ty&& component)
			{
				return m_pRegistry->emplace<_Ty>(entity, std::forward<_Ty>(component));
			}

			template<class _Ty>
			_Ty& GetComponent(entt::entity entity)
			{
				return m_pRegistry->get<_Ty>(entity);
			}
		private:
			nlohmann::json SerializeEntity(entt::entity entity);
			void DeserializeEntity(const nlohmann::json& j);
		private:
			utl::hash_map<entt::entity, class CGameObject*> m_gameObjects;
			std::unique_ptr<entt::registry> m_pRegistry{};
			entt::entity m_root{};
		};
	}
}