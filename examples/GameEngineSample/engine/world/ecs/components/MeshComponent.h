#pragma once

#include "../../../rendering/GeometryBufferInterface.h"
#include <embed/shaders/includes/shlib.h>

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FMeshComponent
			{
				DECLARE_COMPONENT(FMeshComponent, icon::shape, "Mesh renderer")

				uint32_t mesh{ ~0u };
				int32_t skin{ -1 };
				bool castShadows{ true };
			};
		}
	}
}

REFL_AUTO(
	type(engine::world::ecs::FMeshComponent),
	field(castShadows)
)