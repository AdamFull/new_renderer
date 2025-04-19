#pragma once

#include "BaseSystem.h"

namespace engine::world::ecs {
	class AnimationSystem final : public ISystem {
	public:
		AnimationSystem();
		virtual ~AnimationSystem() override = default;

		void __create() override;
		void __update(float fDt) override;
	private:
		std::vector<float> m_timers{};
		std::vector<uint64_t> m_animationIndeces{};
	};
}