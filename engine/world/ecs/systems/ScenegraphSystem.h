#pragma once

#include "BaseSystem.h"
#include <world/ecs/components/fwd.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CScenegraphSystem : public ISystem
			{
			public:
				virtual ~CScenegraphSystem() override = default;

				void __create() override;
				void __update(float fDt) override;
			private:
				static void buildHierarchy(entt::registry* registry, uint32_t node);
				static void initMatrix(FTransformComponent* transform);
				static void initMatrices(entt::registry* registry, uint32_t node);
				static void calculateMatrices(entt::registry* registry, uint32_t node);
			};
		}
	}
}