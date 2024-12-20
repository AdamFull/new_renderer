#pragma once

#include "BaseSystem.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CAnimationSystem : public ISystem
			{
			public:
				CAnimationSystem();
				virtual ~CAnimationSystem() override = default;

				void __Create() override;
				void __Update(float fDt) override;
			private:
				std::vector<float> m_timers{};
				std::vector<uint64_t> m_animationIndeces{};
			};
		}
	}
}