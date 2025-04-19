#include "AnimationSystem.h"

#include "../../EntityManager.h"
#include "../components/components.h"

#include <edge/util/utl_profiler.h>
#include <execution>

#define USE_OLD_ANIMATION_SYSTEM
//#define USE_STD_MULTITHREADING

namespace engine::world::ecs {
	auto q_linear(const std::vector<glm::vec4>& positions, uint32_t index, const float interp) -> glm::quat {
		auto& prev = positions[index];
		auto& next = positions[index + 1];

		glm::quat prevq{ prev.x, prev.y, prev.z, prev.w };
		glm::quat nextq{ next.x, next.y, next.z, next.w };
		return glm::normalize(glm::slerp(prevq, nextq, interp));
	}

	auto v_linear(const std::vector<glm::vec4>& positions, uint32_t index, const float interp) -> glm::vec3 {
		auto& prev = positions[index];
		auto& next = positions[index + 1];
		return glm::mix(prev, next, interp);
	}

	auto q_step(const std::vector<glm::vec4>& positions, uint32_t index, const float interp) -> glm::quat {
		auto& pos = positions[index + 1];
		return glm::normalize(glm::quat{ pos.x, pos.y, pos.z, pos.w });
	}

	auto v_step(const std::vector<glm::vec4>& positions, uint32_t index, const float interp) -> glm::vec3 {
		return positions[index + 1];
	}

	auto cubicspline(const std::vector<glm::vec4>& positions, uint32_t index, const float interp) -> glm::vec4 {
		auto t = interp;
		auto t2 = t * t;
		auto t3 = t2 * t;

		auto prevIndex = index * 3;
		auto nextIndex = (index + 1) * 3;

		auto& prev = positions[prevIndex + 1];
		auto prevTangent = positions[prevIndex + 2];
		auto& next = positions[nextIndex + 1];
		auto nextTangent = positions[nextIndex];

		return (2 * t3 - 3 * t2 + 1) * prev + (t3 - 2 * t2 + t) * prevTangent + (-2 * t3 + 3 * t2) * next + (t3 - t2) * nextTangent;
	}

	auto v_interpolate(EInterpolationType interpolation, const std::vector<glm::vec4>& positions, uint32_t index, const float interp) -> glm::vec3 {
		if (interpolation == EInterpolationType::eLinear)
			return v_linear(positions, index, interp);
		else if (interpolation == EInterpolationType::eStep)
			return v_step(positions, index, interp);
		else if (interpolation == EInterpolationType::eCubicSpline)
			return cubicspline(positions, index, interp);

		return {};
	}

	auto q_interpolate(EInterpolationType interpolation, const std::vector<glm::vec4>& positions, uint32_t index, const float interp) -> glm::quat {
		if (interpolation == EInterpolationType::eLinear)
			return q_linear(positions, index, interp);
		else if (interpolation == EInterpolationType::eStep)
			return q_step(positions, index, interp);
		else if (interpolation == EInterpolationType::eCubicSpline)
		{
			auto tmp = cubicspline(positions, index, interp);

			glm::quat q;
			q.x = tmp.x;
			q.y = tmp.y;
			q.z = tmp.z;
			q.w = tmp.w;
			return glm::normalize(q);
		}

		return {};
	}

	AnimationSystem::AnimationSystem() {
	}

	void AnimationSystem::__create() {
	}

	void AnimationSystem::__update(float fDt) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto& registry = EntityManager::get_registry();
		auto root = EntityManager::get_root();

		auto& root_component = EntityManager::get_component<RootComponent>(root);

		if (m_animationIndeces.size() != root_component.animations.size()) {
			m_timers.resize(root_component.animations.size());

			m_animationIndeces.resize(root_component.animations.size());
			std::iota(m_animationIndeces.begin(), m_animationIndeces.end(), 0ull);
		}

#ifdef USE_STD_MULTITHREADING
		std::for_each(std::execution::par, m_animationIndeces.begin(), m_animationIndeces.end(),
			[&](const auto& idx) {
#else
		for (const auto& idx : m_animationIndeces) {
#endif
			auto& animation = root_component.animations[idx];
			auto& time = m_timers[idx];

			time += fDt;
			if (time > animation.end) {
				// TODO: throw here on animation finished event
				time -= animation.end;
			}

			for (auto& channel : animation.channels) {
				auto& sampler = animation.samplers[channel.samplerIndex];

				for (auto i = 0; i < sampler.inputs.size() - 1; i++) {
					if ((time >= sampler.inputs[i]) && (time <= sampler.inputs[i + 1])) {
						float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);

						if (u <= 1.0f) {
							auto& transform = registry.get<TransformComponent>(channel.node);
#ifdef USE_OLD_ANIMATION_SYSTEM
							switch (channel.path) {
							case EPathType::eTranslation: { transform.lposition = v_interpolate(sampler.interpolation, sampler.outputsVec4, i, u); } break;
							case EPathType::eScale: { transform.lscale = v_interpolate(sampler.interpolation, sampler.outputsVec4, i, u); } break;
							case EPathType::eRotation: { transform.lrotation = q_interpolate(sampler.interpolation, sampler.outputsVec4, i, u); } break;
							}
#else
							glm::vec3 position{ 1.f }, scale{ 1.f };
							glm::quat rotation{ 1.f, 0.f, 0.f, 0.f };

							switch (channel.path) {
							case EPathType::eTranslation: { position = v_interpolate(sampler.interpolation, sampler.outputsVec4, i, u); } break;
							case EPathType::eScale: { scale = v_interpolate(sampler.interpolation, sampler.outputsVec4, i, u); } break;
							case EPathType::eRotation: { rotation = q_interpolate(sampler.interpolation, sampler.outputsVec4, i, u); } break;
							}

							glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale);

							transform.gmodel = transform.gmodel * model;
#endif // USE_OLD_ANIMATION_SYSTEM
						}
					}
				}
			}
		}
#ifdef USE_STD_MULTITHREADING
			);
#endif
	}
}