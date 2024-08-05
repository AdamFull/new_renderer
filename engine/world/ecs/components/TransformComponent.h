#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FTransformComponent : public IComponent
			{
				DECLARE_COMPONENT(FTransformComponent, icon::transform, "Transform")

				// Local transform
				glm::vec3 lposition{ 0.f };
				glm::quat lrotation{ 1.f, 0.f, 0.f, 0.f };
				glm::vec3 lscale{ 1.f };

				// Global transform
				glm::vec3 gposition{ 0.f };
				glm::quat grotation{ 1.f, 0.f, 0.f, 0.f };
				glm::vec3 gscale{ 1.f };

				glm::mat4 gmodel{ 1.f };
				glm::mat4 lmodel{ 1.f };
				glm::mat4 oldmodel{ 1.f };
				glm::mat4 normal{ 1.f };

				DECLARE_FIELDS(
					FDECL(lposition, "Position", DragField(0.01f, -FLT_MAX, FLT_MAX)),
					FDECL(lrotation, "Rotation", DragField(0.01f, -1.f, 1.f)),
					FDECL(lscale, "Scale", DragField(0.01f, 0.f, FLT_MAX)),
					FDECL(lmodel, "Model", NoEdit{})
				);
			};
		}
	}
}

REFL_AUTO(
	type(engine::world::ecs::FTransformComponent),
	field(lposition, engine::attr::DragField(-FLT_MAX, FLT_MAX)),
	field(lrotation, engine::attr::DragField(-1.f, 1.f)),
	field(lscale, engine::attr::DragField(0.f, FLT_MAX)),
	field(lmodel, engine::attr::HiddenField())
)