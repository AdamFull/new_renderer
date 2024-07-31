#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FSpotLightComponent : public  IComponent
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

				DECLARE_FIELDS(
					FDECL(color, "Color", ColorEditField()),
					FDECL(target, "Target", DragField(0.f, 0.f, 0.01f)),
					FDECL(intencity, "Intencity", DragField(0.f, 1000.f, 0.01f)),
					FDECL(innerAngle, "Inner angle", DragField(0.f, 3.14f, 0.01f)),
					FDECL(outerAngle, "Outer angle", DragField(0.f, 3.14f, 0.01f)),
					FDECL(toTarget, "To target", ToggleField()),
					FDECL(castShadows, "Cast chadows", ToggleField())
				)
			};
		}
	}
}