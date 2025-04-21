#pragma once

#include "Component.h"

namespace engine::world::ecs {
	struct SpotLightComponent {
		static constexpr std::string_view _name{ "Spot Light" };
		static constexpr std::string_view _name_with_icon = edge::join_v<icon::spot_light, _name>;

		glm::vec3 color{ 1.f };
		glm::vec3 target{ 0.f, 0.f, 1.f };
		float intencity{ 1.f };
		float innerAngle{ 0.f };
		float outerAngle{ 0.78f };
		bool toTarget{ false };
		int shadowIndex{ -1 };
		bool castShadows{ true };

		uint32_t shadow_caster{ ~0u };
	};
}

REFL_TYPE(engine::world::ecs::SpotLightComponent,
	engine::attr::Name(engine::world::ecs::SpotLightComponent::_name_with_icon))
REFL_FIELD(color, engine::attr::Name("Color"), engine::attr::ColorEdit())
REFL_FIELD(target, engine::attr::Name("Target Direction"), engine::attr::Range(0.f, 0.f))
REFL_FIELD(intencity, engine::attr::Name("Intencity"), engine::attr::Range(0.f, 0.f))
REFL_FIELD(innerAngle, engine::attr::Name("Inner Angle"), engine::attr::Range(0.f, 3.14f))
REFL_FIELD(outerAngle, engine::attr::Name("Outer Angle"), engine::attr::Range(0.f, 3.14f))
REFL_FIELD(toTarget, engine::attr::Name("Look At Target"))
REFL_FIELD(castShadows, engine::attr::Name("Cast Shadows"))
REFL_END