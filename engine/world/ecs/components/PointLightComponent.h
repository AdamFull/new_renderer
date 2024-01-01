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

				size_t frustumID{ ~0ull };
			};
		}
	}
}