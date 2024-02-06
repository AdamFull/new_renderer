#pragma once

#include <utils/stopwatch.h>

namespace engine
{
	namespace world
	{
		class CSceneGraph;

		namespace ecs
		{
			class ISystem
			{
			public:
				virtual ~ISystem() = default;

				virtual void create();
				void update(f32 dt);

			protected:
				virtual void __create() = 0;
				virtual void __update(f32 fDf) = 0;

				CSceneGraph* m_pSceneGraph{ nullptr };
				utl::utils::stopwatch m_stopWatch;
				f32 m_createTime{ 0.f }, m_updateTime{ 0.f };
			};
		}
	}
}