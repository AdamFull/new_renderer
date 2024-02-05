#pragma once

#include <standart_library.h>

namespace engine
{
	namespace world
	{
		class CSceneGraph
		{
		public:
			CSceneGraph();

			entt::entity createRoot(const stl::string& name);
			entt::entity createNode(const stl::string& name);
			void destroyNode(entt::entity node);
			void attachChild(entt::entity parent, entt::entity child);
			void detachChild(entt::entity parent, entt::entity child);
			void detachChild(entt::entity child);
			entt::entity duplicateNode(entt::entity child);
			void parentExchange(entt::entity new_parent, entt::entity child);

			const stl::unique_ptr<entt::registry>& getRegistry() const;
			entt::entity getRoot() const;

			template<class _Ty>
			_Ty& addComponent(entt::entity entity, _Ty&& component)
			{
				return m_pRegistry->emplace<_Ty>(entity, stl::forward<_Ty>(component));
			}

			template<class _Ty>
			_Ty& getComponent(entt::entity entity)
			{
				return m_pRegistry->get<_Ty>(entity);
			}
		private:
			stl::unique_ptr<entt::registry> m_pRegistry{};
			entt::entity m_root{};
		};
	}
}