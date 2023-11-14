#pragma once

#include <entt/entt.hpp>

namespace engine
{
	namespace world
	{
		class CSceneGraph
		{
		public:
			CSceneGraph();

			entt::entity createNode(const std::string& name);
			void destroyNode(entt::entity node);
			void attachChild(entt::entity parent, entt::entity child);
			void detachChild(entt::entity parent, entt::entity child);
			void detachChild(entt::entity child);
			entt::entity duplicateNode(entt::entity child);
			void parentExchange(entt::entity new_parent, entt::entity child);
		private:
			std::unique_ptr<entt::registry> m_pRegistry{};
		};
	}
}