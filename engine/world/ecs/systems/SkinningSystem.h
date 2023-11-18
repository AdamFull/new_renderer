#pragma once

#include <world/ecs/systems/BaseSystem.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CSkinningSystem : public ISystem
			{
			public:
				CSkinningSystem(CSceneGraph* scenegraph);
				virtual ~CSkinningSystem() override = default;

				void __create() override;
				void __update(float fDt) override;
			};
		}
	}
}