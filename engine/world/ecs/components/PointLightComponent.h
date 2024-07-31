#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FPointLightComponent : public IComponent
			{
				DECLARE_COMPONENT(FPointLightComponent, icon::point_light, "Point light")

				glm::vec3 color{ 1.f };
				f32 intencity{ 1.f };
				f32 radius{ 1.f };
				int shadowIndex{ -1 };
				bool castShadows{ true };

				DECLARE_FIELDS(
					FDECL(color, "Color", ColorEditField()),
					FDECL(intencity, "Intencity", DragField(0.f, 1000.f, 0.01f)),
					FDECL(radius, "Radius", DragField(0.f, 0.f, 0.01f)),
					FDECL(castShadows, "Cast shadows", ToggleField())
				)
			};
		}
	}
}