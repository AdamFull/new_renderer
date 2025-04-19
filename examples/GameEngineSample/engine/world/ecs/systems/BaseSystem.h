#pragma once

#include <entt/entt.hpp>
#include <edge/util/utl_stopwatch.hpp>
#include <edge/runtime/rt_input_events.h>

namespace engine::world::ecs {
	class ISystem {
	public:
		virtual ~ISystem() = default;

		virtual void create();
		void update(float dt);

	protected:
		virtual void __create() = 0;
		virtual void __update(float fDf) = 0;
	private:
		edge::utl::Stopwatch stopwatch{};
		float create_time{ 0 };
		float update_time{ 0 };
	};
}