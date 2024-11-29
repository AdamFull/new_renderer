#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FTransformComponent
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

				uint32_t node_index{};
			};
		}
	}
}

REFL_AUTO(
	type(engine::world::ecs::FTransformComponent),
	field(lposition, engine::attr::Range(-FLT_MAX, FLT_MAX)),
	field(lrotation, engine::attr::Range(0.f, 0.f)),
	field(lscale, engine::attr::Range(0.f, FLT_MAX)),
	field(lmodel, engine::attr::HiddenField())
)