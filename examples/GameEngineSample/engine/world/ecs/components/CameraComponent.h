#pragma once

#include "Component.h"

namespace engine::world::ecs {
	enum class ECameraType {
		eOrthographic,
		ePerspective
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(ECameraType,
		{
			{ECameraType::eOrthographic, "ortho"},
			{ECameraType::ePerspective, "perspective"}
		})

	struct FPupa {
		static constexpr std::string_view _name{ "Pupa" };
		static constexpr std::string_view _name_with_icon = edge::join_v<icon::shape, _name>;

		float biba{ 45.f };
		float boba{ 0.1f };
	};

	struct FCameraComponent {
		static constexpr std::string_view _name{ "Camera" };
		static constexpr std::string_view _name_with_icon = edge::join_v<icon::camera, _name>;

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

		uint32_t frustumID{ ~0u };

		std::vector<FPupa> pupas;
	};
}

EDGE_REGISTER_ENUM_CLASS(engine::world::ecs, ECameraType,
	EDGE_ENUM_META(engine::world::ecs::ECameraType::eOrthographic, "Orthographic"),
	EDGE_ENUM_META(engine::world::ecs::ECameraType::ePerspective, "Perspective")
);

REFL_TYPE(engine::world::ecs::FPupa, 
	engine::attr::Name(engine::world::ecs::FPupa::_name_with_icon))
REFL_FIELD(biba, engine::attr::Name("Biba"), engine::attr::Range(0.f, 0.f))
REFL_FIELD(boba, engine::attr::Name("Boba"), engine::attr::Range(0.f, 0.f))
REFL_END

REFL_TYPE(engine::world::ecs::FCameraComponent, 
	engine::attr::Name(engine::world::ecs::FCameraComponent::_name_with_icon))
REFL_FIELD(ctype, engine::attr::Name("Camera Type"))
REFL_FIELD(fieldOfView, engine::attr::Name("Field Of View"), engine::attr::Range(0.f, 180.f))
REFL_FIELD(nearPlane, engine::attr::Name("Near Plane"), engine::attr::Range(0.f, FLT_MAX))
REFL_FIELD(farPlane, engine::attr::Name("Far Plane"), engine::attr::Range(0.f, FLT_MAX))
REFL_FIELD(aspectRatio, engine::attr::Name("Aspect Ratio"), engine::attr::Range(0.f, FLT_MAX))
REFL_FIELD(xmag, engine::attr::Range(0.f, 0.f))
REFL_FIELD(ymag, engine::attr::Range(0.f, 0.f))
REFL_FIELD(pupas, engine::attr::Name("Vector test"))
REFL_END