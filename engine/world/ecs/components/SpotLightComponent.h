#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FSpotLightComponent
			{
				DECLARE_COMPONENT(FSpotLightComponent, icon::spot_light, "Spot light")

				glm::vec3 color{ 1.f };
				glm::vec3 target{ 0.f, 0.f, 1.f };
				f32 intencity{ 1.f };
				f32 innerAngle{ 0.f };
				f32 outerAngle{ 0.78f };
				bool toTarget{ false };
				int shadowIndex{ -1 };
				bool castShadows{ true };
			};
		}
	}
}

REFL_AUTO(
	type(engine::world::ecs::FSpotLightComponent),
	field(color, engine::attr::ColorField()),
	field(target, engine::attr::Range(0.f, 0.f)),
	field(intencity, engine::attr::Range(0.f, 0.f)),
	field(innerAngle, engine::attr::Range(0.f, 3.14f)),
	field(outerAngle, engine::attr::Range(0.f, 3.14f)),
	field(toTarget),
	field(castShadows)
)