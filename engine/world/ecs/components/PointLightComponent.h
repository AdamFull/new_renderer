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
				f32 intencity{ 1.f };
				f32 radius{ 1.f };
				int shadowIndex{ -1 };
				bool castShadows{ true };
			};
		}
	}
}