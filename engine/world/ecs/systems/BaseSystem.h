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

				virtual void Create();
				void Update(f32 dt);

			protected:
				virtual void __Create() = 0;
				virtual void __Update(f32 fDf) = 0;

				CSceneGraph* m_pSceneGraph{ nullptr };
				utl::utils::stopwatch m_stopWatch;
				f32 m_createTime{ 0.f }, m_updateTime{ 0.f };
			};
		}
	}
}