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
				DECLARE_COMPONENT(FDirectionalLightComponent)

				glm::vec3 color{ 1.f };
				f32 intencity{ 1.f };
				int shadowIndex{ -1 };
				bool castShadows{ true };

				DECLARE_FIELDS(
					FDECL(color, ColorEditField()),
					FDECL(intencity, DragField(0.f, 1000.f, 0.01f)),
					FDECL(castShadows, ToggleField())
				)
			};
		}
	}
}