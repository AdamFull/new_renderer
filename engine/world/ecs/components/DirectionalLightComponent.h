#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FDirectionalLightComponent : public IComponent
			{
				DECLARE_COMPONENT(FDirectionalLightComponent, icon::directional_light, "Directional light")

				glm::vec3 color{ 1.f };
				f32 intencity{ 1.f };
				int shadowIndex{ -1 };
				bool castShadows{ true };

				DECLARE_FIELDS(
					FDECL(color, "Color", ColorEditField()),
					FDECL(intencity, "Intencity", DragField(0.01f, 0.f, 1000.f)),
					FDECL(castShadows, "Cast shadows", ToggleField())
				)
			};
		}
	}
}

REFL_AUTO(
	type(engine::world::ecs::FDirectionalLightComponent),
	field(color, engine::attr::ColorField()),
	field(intencity, engine::attr::DragField(0.f, 0.f)),
	field(castShadows, engine::attr::ToggleField())
)