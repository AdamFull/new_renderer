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
				DECLARE_COMPONENT(FSpotLightComponent)

				glm::vec3 color{ 1.f };
				glm::vec3 target{ 0.f, 0.f, 1.f };
				f32 intencity{ 1.f };
				f32 innerAngle{ 0.f };
				f32 outerAngle{ 0.78f };
				bool toTarget{ false };
				int shadowIndex{ -1 };
				bool castShadows{ true };

				DECLARE_FIELDS(
					FDECL(color, ColorEditField()),
					FDECL(target, DragField(0.f, 0.f, 0.01f)),
					FDECL(intencity, DragField(0.f, 1000.f, 0.01f)),
					FDECL(innerAngle, DragField(0.f, 3.14f, 0.01f)),
					FDECL(outerAngle, DragField(0.f, 3.14f, 0.01f)),
					FDECL(toTarget, ToggleField()), 
					FDECL(castShadows, ToggleField())
				)
			};
		}
	}
}