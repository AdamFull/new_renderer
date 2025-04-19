#pragma once

#include "Component.h"

namespace engine::world::ecs {
	enum class EPathType {
		eTranslation,
		eRotation,
		eScale
	};

	struct FAnimationChannel {
		EPathType path{};
		uint32_t samplerIndex{ 0u };
		entt::entity node{ entt::null };
	};

	enum class EInterpolationType {
		eLinear,
		eStep,
		eCubicSpline
	};

	struct FAnimationSampler {
		EInterpolationType interpolation;
		std::vector<float> inputs;
		std::vector<glm::vec4> outputsVec4;
	};

	struct FMeshAnimation {
		std::string name;
		std::vector<FAnimationSampler> samplers;
		std::vector<FAnimationChannel> channels;
		float start = (std::numeric_limits<float>::max)();
		float end = (std::numeric_limits<float>::min)();
	};

	struct FMeshSkin {
		std::string name;
		entt::entity root{ entt::null };
		std::vector<glm::mat4> inverseBindMatrices;
		std::vector<entt::entity> joints;
		std::vector<glm::mat4> jointMatrices;
		std::vector<uint64_t> indices{};
	};

	struct FRootComponent {
		static constexpr std::string_view _name{ "Root" };
		static constexpr std::string_view _name_with_icon = edge::join_v<icon::entity, _name>;

		std::vector<FMeshAnimation> animations;
		std::vector<FMeshSkin> skins;
		std::unordered_map<uint32_t, uint32_t> skinRemap{};

		entt::entity entity;
	};
}

REFL_TYPE(engine::world::ecs::FRootComponent, 
	engine::attr::Name(engine::world::ecs::FRootComponent::_name_with_icon))
REFL_END