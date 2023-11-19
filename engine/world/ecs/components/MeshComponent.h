#pragma once

#define MAX_LEVEL_OF_DETAIL 4

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FPrimitiveLOD
			{
				size_t beginIndex, indexCount;
				size_t beginVertex, vertexCount;
			};

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

				std::array<FPrimitiveLOD, MAX_LEVEL_OF_DETAIL> lods{};

				size_t material{};
			};

			struct FStaticMeshComponent
			{
				std::vector<FMeshPrimitive> primitives{};
				size_t vboId{ ~0ull };
				bool castShadows{ true };
			};

			struct FDynamicMeshComponent : public FStaticMeshComponent
			{
				int32_t skin{ -1 };
			};
		}
	}
}