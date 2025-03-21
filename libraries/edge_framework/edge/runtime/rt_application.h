#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>

#include "rt_input_events.h"

#include "../fs/fs_virtual_filesystem.h"

namespace edge {
	class Window;
	class Application;

	struct ApplicationCreateInfo {
		std::string name;
		std::function<std::unique_ptr<Application>()> app_allocator;
	};

	struct ApplicationOptions {
		bool benchmark_enabled;
		Window* window;
	};

	class Application {
	public:
		Application() = default;

		virtual ~Application() = default;

		/**
		* @brief Prepares the application for execution
		*/
		virtual auto prepare(const ApplicationOptions& options) -> bool;

		/**
		 * @brief Updates the application 50 times per second
		 * @param delta_time Fixed time tick
		 */
		virtual void fixed_update(float delta_time) {}

		/**
		 * @brief Updates the application
		 * @param delta_time The time since the last update
		 */
		virtual void update(float delta_time) {}

		/**
		 * @brief Handles cleaning up the application
		 */
		virtual void finish() {}

		virtual auto on_resize(uint32_t width, uint32_t height) -> bool {
			return true;
		}

		virtual void on_input_event(const InputEvent& input_event) {}

		inline auto should_close() const noexcept -> bool {
			return requested_close;
		}

		// request the app to close
		// does not guarantee that the app will close immediately
		inline void close() {
			requested_close = true;
		}

		void set_name(std::string_view _name) {
			name = _name;
		}

		auto get_name() const noexcept -> std::string const& {
			return name;
		}

	protected:
		Window* window;

		std::string name;
		bool lock_simulation_speed{ false };
	private:
		bool requested_close{ false };
	};
}