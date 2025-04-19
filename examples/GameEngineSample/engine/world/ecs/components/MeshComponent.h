#pragma once

#include "../../../rendering/GeometryBufferInterface.h"
#include <assets/shaders/includes/shlib.h>

#include "Component.h"

namespace engine::world::ecs {
	struct MeshComponent {
		static constexpr std::string_view _name{ "Mesh" };
		static constexpr std::string_view _name_with_icon = edge::join_v<icon::shape, _name>;

		uint32_t mesh{ ~0u };
		int32_t skin{ -1 };
		bool castShadows{ true };
	};
}

REFL_TYPE(engine::world::ecs::MeshComponent,
	engine::attr::Name(engine::world::ecs::MeshComponent::_name_with_icon))
REFL_FIELD(castShadows, engine::attr::Name("Cast Shadows"))
REFL_END