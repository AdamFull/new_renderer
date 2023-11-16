#pragma once

#include <array>
#include <vector>

#include <glm/glm.hpp>

#define MAX_LEVEL_OF_DETAIL 4

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FPrimitiveLOD
			{
				size_t begin_index, index_count;
				size_t begin_vertex, vertex_count;
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
				} dimensions;

				void setDimensions(glm::vec3 min, glm::vec3 max)
				{
					dimensions.min = min;
					dimensions.max = max;
					dimensions.size = max - min;
					dimensions.center = (min + max) / 2.0f;
					dimensions.radius = glm::distance(min, max) / 2.0f;
				}

				std::array<FPrimitiveLOD, MAX_LEVEL_OF_DETAIL> lods{};

				size_t material{};
				bool castShadows{ false };
			};

			struct FStaticMeshComponent
			{
				std::vector<FMeshPrimitive> primitives{};
			};

			struct FDynamicMeshComponent : public FStaticMeshComponent
			{
				int32_t skin{ -1 };
				uint32_t head{ ~0u };
			};
		}
	}
}