#pragma once

#include <container/hash_map.h>

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
				entt::entity node{ entt::null };
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
				utl::vector<float> inputs;
				utl::vector<glm::vec4> outputsVec4;
			};

			struct FMeshAnimation
			{
				std::string name;
				utl::vector<FAnimationSampler> samplers;
				utl::vector<FAnimationChannel> channels;
				float start = (std::numeric_limits<float>::max)();
				float end = (std::numeric_limits<float>::min)();
			};

			struct FMeshSkin
			{
				std::string name;
				entt::entity root{ entt::null };
				utl::vector<glm::mat4> inverseBindMatrices;
				utl::vector<entt::entity> joints;
				utl::vector<glm::mat4> jointMatrices;
				utl::vector<u64> indices{};
			};

			struct FRootComponent
			{
				utl::vector<FMeshAnimation> animations;
				utl::vector<FMeshSkin> skins;
				utl::hash_map<uint32_t, uint32_t> skinRemap{};
			};
		}
	}
}