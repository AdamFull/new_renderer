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
				struct FDimensions
				{
					glm::vec3 min = glm::vec3(FLT_MAX);
					glm::vec3 max = glm::vec3(-FLT_MAX);
					glm::vec3 size;
					glm::vec3 center;
					float radius;
				} dims;

				void setDimensions(glm::vec3 min, glm::vec3 max)
				{
					dims.min = min;
					dims.max = max;
					dims.size = max - min;
					dims.center = (min + max) / 2.0f;
					dims.radius = glm::distance(min, max) / 2.0f;
				}

				render::VertexRange vertex_range{};

				uint32_t material{ ~0u };
				uint32_t vbo{};

				utl::vector<shlib::FGeometryDrawBounds> bounds{};
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