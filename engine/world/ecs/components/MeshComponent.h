#pragma once

#include <array>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#define MAX_LEVEL_OF_DETAIL 4

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			enum class EPathType
			{
				eTranslation,
				eRotation,
				eScale
			};

			struct FAnimationChannel
			{
				EPathType path{};
				uint32_t samplerIndex{ 0u };
				uint32_t node{ ~0u };
			};

			enum class EInterpolationType
			{
				eLinear,
				eStep,
				eCubicSpline
			};

			struct FAnimationSampler
			{
				EInterpolationType interpolation;
				std::vector<float> inputs;
				std::vector<glm::vec4> outputsVec4;
			};

			struct FMeshAnimation
			{
				std::string name;
				std::vector<FAnimationSampler> samplers;
				std::vector<FAnimationChannel> channels;
				float start = std::numeric_limits<float>::max();
				float end = std::numeric_limits<float>::min();
			};

			struct FMeshSkin
			{
				std::string name;
				uint32_t root{ ~0u };
				std::vector<glm::mat4> inverseBindMatrices;
				std::vector<uint32_t> joints;
				std::array<glm::mat4, 128> jointMatrices;
			};

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