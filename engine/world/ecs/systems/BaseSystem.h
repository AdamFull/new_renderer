#pragma once

#include <utils/stopwatch.h>
#include <engine/runtime/InputEvents.h>

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
				void Update(f32 dt);

				virtual void OnInputEvent(const InputEvent& input_event);

			protected:
				virtual void __Create() = 0;
				virtual void __Update(f32 fDf) = 0;

				utl::utils::stopwatch m_stopWatch;
				f32 m_createTime{ 0.f }, m_updateTime{ 0.f };
			};
		}
	}
}