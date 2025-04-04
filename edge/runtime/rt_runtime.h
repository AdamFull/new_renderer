#pragma once

#include "rt_application.h"
#include "rt_window.h"
#include "../util/Performance.h"

namespace edge {
	class IRuntimeContext;

	enum ExitCode {
		Success = 0, /* App executed as expected */
		Close,       /* App has been requested to close at initialization */
		FatalError   /* App encountered an unexpected error */
	};

	class Runtime {
	public:
		Runtime(const IRuntimeContext& context);

		virtual ~Runtime() = default;

		/**
		 * @brief Initialize the platform
		 * @return An exit code representing the outcome of initialization
		 */
		virtual auto initialize() -> int32_t;

		/**
		 * @brief Handles the main update and render loop
		 * @return An exit code representing the outcome of the loop
		 */
		auto main_loop() -> int32_t;

		/**
		 * @brief Handles the update and render of a frame.
		 * Called either from main_loop(), or from a platform-specific
		 * frame-looping mechanism, typically tied to platform screen refeshes.
		 * @return An exit code representing the outcome of the loop
		 */
		auto main_loop_frame() -> int32_t;

		/**
		 * @brief Runs the application for one frame
		 */
		void update();

		/**
		 * @brief Terminates the platform and the application
		 * @param code Determines how the platform should exit
		 */
		virtual void terminate(int32_t code);

		/**
		 * @brief Requests to close the platform at the next available point
		 */
		virtual void close();

		virtual void on_resize(uint32_t width, uint32_t height);

		virtual void on_input_event(const InputEvent& input_event);

		auto get_window() const noexcept -> Window& {
			return *window;
		}

		auto get_app() const noexcept -> Application& {
			return *app;
		}

		auto get_app() noexcept -> Application& {
			return *app;
		}

		void set_last_error(const std::string& error);

		void set_focus(bool focused) noexcept {
			this->focused = focused;
		}

		void request_application(ApplicationCreateInfo* app_ci) noexcept {
			app_create_info = app_ci;
		}

		auto app_requested() const noexcept -> bool {
			return app_create_info != nullptr;
		}

		auto start_app() -> bool;

		// Force the application to always render even if it is not in focus
		void force_render(bool should_always_render) noexcept {
			always_render = should_always_render;
		}

		void disable_input_processing() noexcept {
			process_input_events = false;
		}

		void set_window_properties(const Window::OptionalProperties& properties);
	protected:
		std::unique_ptr<Window> window{ nullptr };

		std::unique_ptr<Application> app{ nullptr };

		ApplicationCreateInfo* app_create_info{ nullptr };

		/**
		 * @brief Handles the creation of the window
		 *
		 * @param properties Preferred window configuration
		 */
		virtual void create_window(const Window::Properties& properties) = 0;

		void on_update(float delta_time);
		void on_app_error(const std::string& app_id);
		void on_app_start(const std::string& app_id);
		void on_app_close(const std::string& app_id);
		void on_close();

		const float fixed_delta_time{ 0.02f };
		float accumulated_frame_time{ 0.0f };

		Window::Properties window_properties; /* Source of truth for window state */
		bool always_render{ false };          /* App should always render even if not in focus */
		bool process_input_events{ true };    /* App should continue processing input events */
		bool focused{ true };                 /* App is currently in focus at an operating system level */
		bool close_requested{ false };        /* Close requested */

        std::string internal_resources_path{};
        std::string external_resources_path{};
        std::string internal_data_path{};
        std::string external_data_path{};

	private:
		FPSCounter counter{};
	};
}