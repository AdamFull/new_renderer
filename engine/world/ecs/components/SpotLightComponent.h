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

				void OnEdit() override;

				DECLARE_FIELDS(FDECL(color), FDECL(target), FDECL(intencity), FDECL(innerAngle),
					FDECL(outerAngle), FDECL(toTarget), FDECL(castShadows))
			};
		}
	}
}