#pragma once

#include <functional>
#include <edge/runtime/rt_input_events.h>

namespace e2d {
	using OnSubsystemProcessCb = std::function<void(float)>;

	class ISubsystem {
	public:
		virtual ~ISubsystem() = default;

		virtual auto set_on_process(OnSubsystemProcessCb) -> void = 0;
		virtual auto process(float delta_time) -> void = 0;
		virtual auto on_input_event(const edge::InputEvent& input_event) -> void = 0;
	};
}