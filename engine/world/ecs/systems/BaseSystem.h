#pragma once

#include <entt/entt.hpp>
#include <edge/runtime/rt_input_events.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class ISystem
			{
			public:
				virtual ~ISystem() = default;

				virtual void Create();
				void Update(float dt);

			protected:
				virtual void __Create() = 0;
				virtual void __Update(float fDf) = 0;
			};
		}
	}
}