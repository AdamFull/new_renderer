#pragma once

#include <standart_library.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			enum class ECameraType
			{
				eOrthographic,
				ePerspective
			};

			struct FCameraComponent
			{
				ECameraType type{ ECameraType::ePerspective };

				f32 fieldOfView{ 45.f };
				f32 nearPlane{ 0.1f };
				f32 farPlane{ 1024 };

				f32 xmag{ 0.f };
				f32 ymag{ 0.f };

				glm::mat4 view{ 1.f };
				glm::mat4 projection{ 1.f };

				glm::vec3 up{ 0.f };
				glm::vec3 forward{ 0.f };
				glm::vec3 right{ 0.f };

				bool moved{ true };
				bool active{ true };
				bool controllable{ true };
				bool orbit{ true };

				u64 frustumID{ ~0ull };
			};
		}
	}
}