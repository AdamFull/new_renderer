#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FPointLightComponent
			{
				DECLARE_COMPONENT(FPointLightComponent, icon::point_light, "Point light")

				glm::vec3 color{ 1.f };
				float intencity{ 1.f };
				float radius{ 1.f };
				int shadowIndex{ -1 };
				bool castShadows{ true };

				std::array<uint32_t, 6u> shadow_casters{};
			};
		}
	}
}

REFL_AUTO(
	type(engine::world::ecs::FPointLightComponent),
	field(color, engine::attr::ColorField()),
	field(intencity, engine::attr::Range(0.f, 0.f)),
	field(radius, engine::attr::Range(0.f, FLT_MAX)),
	field(castShadows)
)