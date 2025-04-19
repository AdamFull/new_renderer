#pragma once

#include "Component.h"

namespace engine::world::ecs {
	struct BaseComponent {
		static constexpr std::string_view _name{ "Base" };
		static constexpr std::string_view _name_with_icon = edge::join_v<icon::entity, _name>;

		BaseComponent() = default;
		BaseComponent(std::string_view name) : name(name) {}
		std::string name{};
		bool enabled{ true };
		bool visible{ true };
	};
}

REFL_TYPE(engine::world::ecs::BaseComponent,
	engine::attr::Name(engine::world::ecs::BaseComponent::_name_with_icon))
REFL_END