#pragma once

#include <engine/world/ecs/systems/BaseSystem.h>

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
				utl::vector<float> m_timers{};
				utl::vector<u64> m_animationIndeces{};
			};
		}
	}
}