#pragma once

#include <engine/interface/render/VertexStructures.h>
#include <embed/shaders/includes/interop/base.h>

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

				render::FVertexRange vertexRange{};

				size_t materialId{ ~0ull };
				render::FRHIHandle vboId{};

				utl::vector<render::FMeshlet> meshlets{};
				utl::vector<u32> vertices{};
				utl::vector<render::FGeometryDrawBounds> bounds{};
				utl::vector<render::FTriangle> triangles{};
			};

			struct FMeshComponent
			{
				utl::vector<FMeshPrimitive> primitives{};
				int32_t skin{ -1 };
				bool castShadows{ true };
			};
		}
	}
}