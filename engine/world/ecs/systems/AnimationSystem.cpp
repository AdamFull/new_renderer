#include "AnimationSystem.h"

#include <world/ecs/components/components.h>
#include <world/SceneGraph.h>

#include <profiler/Profiler.h>

using namespace engine::world::ecs;

glm::quat q_linear(const std::vector<glm::vec4>& positions, uint32_t index, const float interp)
{
	auto& prev = positions[index];
	auto& next = positions[index + 1];

	glm::quat prevq;
	prevq.x = prev.x;
	prevq.y = prev.y;
	prevq.z = prev.z;
	prevq.w = prev.w;

	glm::quat nextq;
	nextq.x = next.x;
	nextq.y = next.y;
	nextq.z = next.z;
	nextq.w = next.w;

	return glm::normalize(glm::slerp(prevq, nextq, interp));
}

glm::vec3 v_linear(const std::vector<glm::vec4>& positions, uint32_t index, const float interp)
{
	auto& prev = positions[index];
	auto& next = positions[index + 1];
	return glm::mix(prev, next, interp);
}

glm::quat q_step(const std::vector<glm::vec4>& positions, uint32_t index, const float interp)
{
	auto& pos = positions[index + 1];
	glm::quat q;
	q.x = pos.x;
	q.y = pos.y;
	q.z = pos.z;
	q.w = pos.w;

	return glm::normalize(q);
}

glm::vec3 v_step(const std::vector<glm::vec4>& positions, uint32_t index, const float interp)
{
	return positions[index + 1];
}

glm::vec4 cubicspline(const std::vector<glm::vec4>& positions, uint32_t index, const float interp)
{
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

glm::vec3 v_interpolate(EInterpolationType interpolation, const std::vector<glm::vec4>& positions, uint32_t index, const float interp)
{
	if (interpolation == EInterpolationType::eLinear)
		return v_linear(positions, index, interp);
	else if (interpolation == EInterpolationType::eStep)
		return v_step(positions, index, interp);
	else if (interpolation == EInterpolationType::eCubicSpline)
		return cubicspline(positions, index, interp);
}

glm::quat q_interpolate(EInterpolationType interpolation, const std::vector<glm::vec4>& positions, uint32_t index, const float interp)
{
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
}

CAnimationSystem::CAnimationSystem(CSceneGraph* scenegraph)
{
	m_pSceneGraph = scenegraph;
}

void CAnimationSystem::__create()
{
	TRACY_PROFILE_FUNCTION();
}

void CAnimationSystem::__update(float fDt)
{
	TRACY_PROFILE_FUNCTION();

	auto& registry = m_pSceneGraph->getRegistry();
	auto root = m_pSceneGraph->getRoot();

	auto& root_component = m_pSceneGraph->getComponent<FRootComponent>(root);

	// TODO: update here only enabled animattions
	uint32_t timer_index{ 0u };
	for (const auto& animation : root_component.animations)
	{
		auto& time = m_timers[timer_index];

		time += fDt;
		if (time > animation.end)
		{
			// TODO: throw here on animation finished event
			time -= animation.end;
		}

		for (auto& channel : animation.channels)
		{
			auto& sampler = animation.samplers[channel.samplerIndex];

			for (auto i = 0; i < sampler.inputs.size() - 1; i++)
			{
				if ((time >= sampler.inputs[i]) && (time <= sampler.inputs[i + 1]))
				{
					float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);

					if (u <= 1.0f)
					{
						auto& transform = registry->get<FTransformComponent>(channel.node);
						switch (channel.path)
						{
						case EPathType::eTranslation: {
							transform.lposition = v_interpolate(sampler.interpolation, sampler.outputsVec4, i, u);
						} break;
						case EPathType::eScale: {
							transform.lscale = v_interpolate(sampler.interpolation, sampler.outputsVec4, i, u);
						} break;
						case EPathType::eRotation: {
							transform.lrotation = q_interpolate(sampler.interpolation, sampler.outputsVec4, i, u);
						} break;
						}
					}
				}
			}
		}

		timer_index++;
	}
}