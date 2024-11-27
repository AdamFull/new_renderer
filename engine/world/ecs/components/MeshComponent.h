#pragma once

#include <engine/rendering/GeometryBufferInterface.h>
#include <embed/shaders/includes/shlib.h>

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FMeshPrimitive
			{
				uint32_t mesh{ ~0u };
				uint32_t vbo{};
			};

			struct FMeshComponent
			{
				DECLARE_COMPONENT(FMeshComponent, icon::shape, "Mesh renderer")

				utl::vector<FMeshPrimitive> primitives{};
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