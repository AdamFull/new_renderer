#pragma once

#include "Component.h"

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

			NLOHMANN_JSON_SERIALIZE_ENUM(ECameraType,
				{
					{ECameraType::eOrthographic, "ortho"},
					{ECameraType::ePerspective, "perspective"}
				})

			struct FPupa
			{
				DECLARE_COMPONENT(FPupa, icon::shape, "Pupa")

				float biba{ 45.f };
				float boba{ 0.1f };
			};

			struct FCameraComponent
			{
				DECLARE_COMPONENT(FCameraComponent, icon::camera, "Camera")

				ECameraType ctype{ ECameraType::ePerspective };

				float fieldOfView{ 45.f };
				float nearPlane{ 0.1f };
				float farPlane{ 100.f };
				float aspectRatio{ 1.7777777f };

				float xmag{ 0.f };
				float ymag{ 0.f };

				glm::vec3 up{ 0.f };
				glm::vec3 forward{ 0.f };
				glm::vec3 right{ 0.f };

				bool moved{ true };
				bool active{ true };
				bool controllable{ true };
				bool orbit{ true };

				uint64_t frustumID{ ~0ull };

				std::vector<FPupa> pupas;
			};
		}
	}
}

EDGE_REGISTER_ENUM_CLASS(engine::world::ecs, ECameraType,
	EDGE_ENUM_META(engine::world::ecs::ECameraType::eOrthographic, "Orthographic"),
	EDGE_ENUM_META(engine::world::ecs::ECameraType::ePerspective, "Perspective")
);

REFL_AUTO(
	type(engine::world::ecs::FPupa),
	field(biba, engine::attr::Range(0.f, 0.f)),
	field(boba, engine::attr::Range(0.f, 0.f))
)

REFL_AUTO(
	type(engine::world::ecs::FCameraComponent),
	field(ctype),
	field(fieldOfView, engine::attr::Range(0.f, 180.f)),
	field(nearPlane, engine::attr::Range(0.f, FLT_MAX)),
	field(farPlane, engine::attr::Range(0.f, FLT_MAX)),
	field(aspectRatio, engine::attr::Range(0.f, FLT_MAX)),
	field(xmag, engine::attr::Range(0.f, 0.f)),
	field(ymag, engine::attr::Range(0.f, 0.f)),
	field(pupas)
)