#pragma once

#include "Component.h"

namespace engine::world::ecs {
	struct FTransformComponent {
		static constexpr std::string_view _name{ "Transform" };
		static constexpr std::string_view _name_with_icon = edge::join_v<icon::transform, _name>;

		// Local transform
		glm::vec3 lposition{ 0.f };
		glm::quat lrotation{ 1.f, 0.f, 0.f, 0.f };
		glm::vec3 lscale{ 1.f };

		// Global transform
		glm::vec3 gposition{ 0.f };
		glm::quat grotation{ 1.f, 0.f, 0.f, 0.f };
		glm::vec3 gscale{ 1.f };

		glm::mat4 gmodel{ 1.f };
		glm::mat4 lmodel{ 1.f };
		glm::mat4 oldmodel{ 1.f };
		glm::mat4 normal{ 1.f };
	};
}

REFL_TYPE(engine::world::ecs::FTransformComponent, 
	engine::attr::Name(engine::world::ecs::FTransformComponent::_name_with_icon))
REFL_FIELD(lposition, engine::attr::Name("Position"), engine::attr::Range(-FLT_MAX, FLT_MAX))
REFL_FIELD(lrotation, engine::attr::Name("Rotation"), engine::attr::Range(0.f, 0.f))
REFL_FIELD(lscale, engine::attr::Name("Scale"), engine::attr::Range(0.f, FLT_MAX))
REFL_FIELD(lmodel, engine::attr::HiddenField())
REFL_END