#pragma once

#include <entt/fwd.hpp>
#include <utime.hpp>

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

				virtual void create();
				void update(float dt);

			protected:
				virtual void __create() = 0;
				virtual void __update(float fDf) = 0;

				entt::registry* m_pRegistry{ nullptr };
				utl::stopwatch m_stopWatch;
				float m_createTime{ 0.f }, m_updateTime{ 0.f };
			};
		}
	}
}