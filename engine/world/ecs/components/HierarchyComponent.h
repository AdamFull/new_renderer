#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FHierarchyComponent : public IComponent
			{
				DECLARE_COMPONENT(FHierarchyComponent)

				FHierarchyComponent(entt::entity parent) : parent(parent) {}
				entt::entity parent{ entt::null };
				utl::vector<entt::entity> children{};
			};
		}
	}
}