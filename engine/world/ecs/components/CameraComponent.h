#pragma once

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

				float fieldOfView{ 45.f };
				float nearPlane{ 0.1f };
				float farPlane{ 64.f };

				float xmag{ 0.f };
				float ymag{ 0.f };

				glm::vec3 viewPos{ 0.f };
				glm::mat4 view{ 1.f }, invView{ 1.f }, projection{ 1.f }, invProjection{ 1.f };
				glm::uvec2 viewportExtent{};

				glm::vec3 up{ 0.f };
				glm::vec3 forward{ 0.f };
				glm::vec3 right{ 0.f };

				bool moved{ true };
				bool active{ true };
				bool controllable{ true };
				bool orbit{ true };
			};
		}
	}
}