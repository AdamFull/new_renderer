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
				utl::vector<entt::entity> children{};
			};
		}
	}
}