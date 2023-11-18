#pragma once

namespace engine
{
	namespace world
	{
		class CSceneGraph
		{
		public:
			CSceneGraph();

			entt::entity createRoot(const std::string& name);
			entt::entity createNode(const std::string& name);
			void destroyNode(entt::entity node);
			void attachChild(entt::entity parent, entt::entity child);
			void detachChild(entt::entity parent, entt::entity child);
			void detachChild(entt::entity child);
			entt::entity duplicateNode(entt::entity child);
			void parentExchange(entt::entity new_parent, entt::entity child);

			const std::unique_ptr<entt::registry>& getRegistry() const;
			entt::entity getRoot() const;

			template<class _Ty>
			_Ty& addComponent(entt::entity entity, _Ty&& component)
			{
				return m_pRegistry->emplace<_Ty>(entity, std::forward<_Ty>(component));
			}

			template<class _Ty>
			_Ty& getComponent(entt::entity entity)
			{
				return m_pRegistry->get<_Ty>(entity);
			}
		private:
			std::unique_ptr<entt::registry> m_pRegistry{};
			entt::entity m_root{};
		};
	}
}