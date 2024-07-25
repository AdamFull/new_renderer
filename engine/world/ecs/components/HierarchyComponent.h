#pragma once

#include <standart_library.h>
#include <entt/fwd.hpp>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FHierarchyComponent
			{
				FHierarchyComponent(entt::entity parent) : parent(parent) {}
				entt::entity parent{ entt::null };
				utl::vector<entt::entity> children{};
			};
		}
	}
}