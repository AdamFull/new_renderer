#include "BaseSystem.h"

namespace engine::world::ecs {
	void ISystem::create() {
		stopwatch.stop<float>();
		__create();
		create_time = stopwatch.stop<float>();
	}

	void ISystem::update(float dt) {
		stopwatch.stop<float>();
		__update(dt);
		update_time = stopwatch.stop<float>();
	}
}