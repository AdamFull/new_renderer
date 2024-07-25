#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <standart_library.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FDirectionalLightComponent
			{
				glm::vec3 color{ 1.f };
				f32 intencity{ 1.f };
				int shadowIndex{ -1 };
				bool castShadows{ true };
			};
		}
	}
}