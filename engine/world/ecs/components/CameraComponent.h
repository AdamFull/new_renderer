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

			struct FPupa : public IComponent
			{
				DECLARE_COMPONENT(FPupa, icon::shape, "Pupa")

				f32 biba{ 45.f };
				f32 boba{ 0.1f };

				DECLARE_FIELDS(
					FDECL(biba, "Biba", DragField(0.01f, 0.f, 0.f)),
					FDECL(boba, "Boba", DragField(0.01f, 0.f, 0.f))
					)
			};

			struct FCameraComponent : public IComponent
			{
				DECLARE_COMPONENT(FCameraComponent, icon::camera, "Camera")

				ECameraType type{ ECameraType::ePerspective };

				f32 fieldOfView{ 45.f };
				f32 nearPlane{ 0.1f };
				f32 farPlane{ 1024 };
				f32 aspectRatio{ 1.7777777f };

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

				std::vector<FPupa> pupas;

				DECLARE_FIELDS(
					FDECL(type, "Type", DropdownField<2>({ "ortho", "perspective" })),
					FDECL(fieldOfView, "FOV", DragField(0.01f, 0.f, 180.f)),
					FDECL(nearPlane, "Near", DragField(0.01f, 0.01f, 0.f)),
					FDECL(farPlane, "Far", DragField(0.01f, 0.01f, 0.f)),
					FDECL(aspectRatio, "Aspect", DragField(0.01f, 0.01f, 0.f)),
					FDECL(xmag, "XMag", DragField(0.01f, 0.01f, 0.f)),
					FDECL(ymag, "YMag", DragField(0.01f, 0.01f, 0.f)),
					FDECL(pupas, "Pupa vector", ContainerField<FPupa>())
				)

				//
			};
		}
	}
}