#pragma once

#include "Component.h"

namespace engine::world::ecs {
	struct PointLightComponent {
		static constexpr std::string_view _name{ "Point Light" };
		static constexpr std::string_view _name_with_icon = edge::join_v<icon::point_light, _name>;

		glm::vec3 color{ 1.f };
		float intencity{ 1.f };
		float radius{ 1.f };
		int shadowIndex{ -1 };
		bool castShadows{ true };

		std::array<uint32_t, 6u> shadow_casters{};
	};
}

REFL_TYPE(engine::world::ecs::PointLightComponent,
	engine::attr::Name(engine::world::ecs::PointLightComponent::_name_with_icon))
REFL_FIELD(color, engine::attr::Name("Color"), engine::attr::ColorEdit())
REFL_FIELD(intencity, engine::attr::Name("Intencity"), engine::attr::Range(0.f, 0.f))
REFL_FIELD(radius, engine::attr::Name("Radius"), engine::attr::Range(0.f, FLT_MAX))
REFL_FIELD(castShadows, engine::attr::Name("Cast Shadows"))
REFL_END