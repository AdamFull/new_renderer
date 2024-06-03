#pragma once

#include <engine/world/ecs/systems/BaseSystem.h>
#include <engine/world/ecs/components/fwd.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CScenegraphSystem : public ISystem
			{
			public:
				CScenegraphSystem(CSceneGraph* scenegraph);
				virtual ~CScenegraphSystem() override = default;

				void __Create() override;
				void __Update(float fDt) override;
			private:
				static void buildHierarchy(entt::registry* registry, entt::entity node);
				static void initMatrix(FTransformComponent* transform);
				static void initMatrices(entt::registry* registry, entt::entity node);
				static void calculateMatrices(entt::registry* registry, entt::entity node);
			};
		}
	}
}