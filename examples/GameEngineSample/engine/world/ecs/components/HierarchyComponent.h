#pragma once

#include "Component.h"

namespace engine::world::ecs {
	struct FHierarchyComponent {
		static constexpr std::string_view _name{ "Hierarchy" };
		static constexpr std::string_view _name_with_icon = edge::join_v<icon::entity, _name>;

		FHierarchyComponent() {};
		FHierarchyComponent(entt::entity parent) : parent(parent) {}
		entt::entity parent{ entt::null };
		std::vector<entt::entity> children{};
	};
}

REFL_TYPE(engine::world::ecs::FHierarchyComponent, 
	engine::attr::Name(engine::world::ecs::FHierarchyComponent::_name_with_icon))
REFL_END