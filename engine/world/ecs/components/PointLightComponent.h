#pragma once

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FPointLightComponent
			{
				glm::vec3 color{ 1.f };
				float intencity{ 1.f };
				float radius{ 1.f };
				int shadowIndex{ -1 };
				bool castShadows{ false };
			};

			struct FOmniShadowCommit
			{
				alignas(4) float farPlane;
				alignas(4) int index;
			};

			struct FPointLightCommit
			{
				alignas(16) glm::vec3 position;
				alignas(16) glm::vec3 color;
				alignas(4) float intencity;
				alignas(4) float radius;
				alignas(4) int shadowIndex;
				alignas(4) int castShadows;
			};
		}
	}
}