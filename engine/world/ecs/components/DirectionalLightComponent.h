#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FDirectionalLightComponent
			{
				DECLARE_COMPONENT(FDirectionalLightComponent, icon::directional_light, "Directional light")

				glm::vec3 color{ 1.f };
				f32 intencity{ 1.f };
				int shadowIndex{ -1 };
				bool castShadows{ true };
			};
		}
	}
}

REFL_AUTO(
	type(engine::world::ecs::FDirectionalLightComponent),
	field(color, engine::attr::ColorField()),
	field(intencity, engine::attr::Range(0.f, 0.f)),
	field(castShadows)
)