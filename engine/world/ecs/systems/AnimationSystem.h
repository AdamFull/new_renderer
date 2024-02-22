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
				CAnimationSystem(CSceneGraph* scenegraph);
				virtual ~CAnimationSystem() override = default;

				void __create() override;
				void __update(float fDt) override;
			private:
				std::vector<float> m_timers{};
				std::vector<u64> m_animationIndeces{};
			};
		}
	}
}