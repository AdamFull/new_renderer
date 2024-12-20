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

				f32 biba{ 45.f };
				f32 boba{ 0.1f };
			};

			struct FCameraComponent
			{
				DECLARE_COMPONENT(FCameraComponent, icon::camera, "Camera")

				ECameraType ctype{ ECameraType::ePerspective };

				f32 fieldOfView{ 45.f };
				f32 nearPlane{ 0.1f };
				f32 farPlane{ 100.f };
				f32 aspectRatio{ 1.7777777f };

				f32 xmag{ 0.f };
				f32 ymag{ 0.f };

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