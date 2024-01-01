#pragma once

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FSpotLightComponent
			{
				glm::vec3 color{ 1.f };
				glm::vec3 target{ 0.f, 0.f, 1.f };
				float intencity{ 1.f };
				float innerAngle{ 0.f };
				float outerAngle{ 0.78f };
				bool toTarget{ false };
				int shadowIndex{ -1 };
				bool castShadows{ false };

				size_t frustumID{ ~0ull };
			};
		}
	}
}