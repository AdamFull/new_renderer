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

				void OnEdit() override;

				DECLARE_FIELDS(FDECL(color), FDECL(intencity), FDECL(castShadows))
			};
		}
	}
}