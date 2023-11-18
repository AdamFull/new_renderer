#pragma once

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FHierarchyComponent
			{
				entt::entity parent{ entt::null };
				std::vector<entt::entity> children{};
			};
		}
	}
}