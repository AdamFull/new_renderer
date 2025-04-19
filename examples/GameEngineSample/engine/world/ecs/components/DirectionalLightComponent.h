#pragma once

#include "Component.h"

namespace engine::world::ecs {
	struct DirectionalLightComponent {
		static constexpr std::string_view _name{ "Directional Light" };
		static constexpr std::string_view _name_with_icon = edge::join_v<icon::directional_light, _name>;

		glm::vec3 color{ 1.f };
		float intencity{ 1.f };
		int shadowIndex{ -1 };
		bool castShadows{ true };

		std::vector<uint32_t> shadow_casters{};
	};
}

REFL_TYPE(engine::world::ecs::DirectionalLightComponent,
	engine::attr::Name(engine::world::ecs::DirectionalLightComponent::_name_with_icon))
REFL_FIELD(color, engine::attr::Name("Color"), engine::attr::ColorField())
REFL_FIELD(intencity, engine::attr::Name("Intencity"), engine::attr::Range(0.f, 0.f))
REFL_FIELD(castShadows, engine::attr::Name("Cast Shadows"))
REFL_END