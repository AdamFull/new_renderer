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
				float intencity{ 1.f };
				int shadowIndex{ -1 };
				bool castShadows{ true };

				std::vector<uint32_t> shadow_casters{};
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