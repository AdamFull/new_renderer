#pragma once

#include "Application.h"
#include "Window.h"
#include "../filesystem/VirtualFileSystemInterface.h"
#include "../util/Performance.h"

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace edge
{
	class IRuntimeContext;

	enum ExitCode
	{
		Success = 0, /* App executed as expected */
		Close,       /* App has been requested to close at initialization */
		FatalError   /* App encountered an unexpected error */
	};

	class Runtime
	{
	public:
		Runtime(const IRuntimeContext& context);

		virtual ~Runtime() = default;

		/**
		 * @brief Initialize the platform
		 * @return An exit code representing the outcome of initialization
		 */
		virtual auto Initialize() -> int32_t;

		/**
		 * @brief Handles the main update and render loop
		 * @return An exit code representing the outcome of the loop
		 */
		auto MainLoop() -> int32_t;

		/**
		 * @brief Handles the update and render of a frame.
		 * Called either from main_loop(), or from a platform-specific
		 * frame-looping mechanism, typically tied to platform screen refeshes.
		 * @return An exit code representing the outcome of the loop
		 */
		auto MainLoopFrame() -> int32_t;

		/**
		 * @brief Runs the application for one frame
		 */
		void Update();

		/**
		 * @brief Terminates the platform and the application
		 * @param code Determines how the platform should exit
		 */
		virtual void Terminate(int32_t code);

		/**
		 * @brief Requests to close the platform at the next available point
		 */
		virtual void Close();

		virtual void OnResize(uint32_t width, uint32_t height);

		virtual void OnInputEvent(const InputEvent& input_event);

		auto GetWindow() const noexcept -> Window&
		{
			return *window;
		}

		auto GetApp() const noexcept -> Application&
		{
			return *app;
		}

		auto GetApp() noexcept -> Application&
		{
			return *app;
		}

		void SetLastError(const std::string& error);

		void SetFocus(bool focused) noexcept
		{
			this->focused = focused;
		}

		void RequestApplication(ApplicationCreateInfo* app_ci) noexcept
		{
			app_create_info = app_ci;
		}

		auto AppRequested() const noexcept -> bool
		{
			return app_create_info != nullptr;
		}

		auto StartApp() -> bool;

		// Force the application to always render even if it is not in focus
		void ForceRender(bool should_always_render) noexcept
		{
			always_render = should_always_render;
		}

		void DisableInputProcessing() noexcept
		{
			process_input_events = false;
		}

		void SetWindowProperties(const Window::OptionalProperties& properties);
	protected:
		std::unique_ptr<Window> window{ nullptr };
		std::unique_ptr<filesystem::IVirtualFileSystemInterface> filesystem{};

		std::unique_ptr<Application> app{ nullptr };

		ApplicationCreateInfo* app_create_info{ nullptr };

		/**
		 * @brief Handles the creation of the window
		 *
		 * @param properties Preferred window configuration
		 */
		virtual void CreateWindow(const Window::Properties& properties) = 0;

		void OnUpdate(float delta_time);
		void OnAppError(const std::string& app_id);
		void OnAppStart(const std::string& app_id);
		void OnAppClose(const std::string& app_id);
		void OnClose();

		const float fixed_delta_time{ 0.02f };
		float accumulated_frame_time{ 0.0f };

		Window::Properties window_properties; /* Source of truth for window state */
		bool always_render{ false };          /* App should always render even if not in focus */
		bool process_input_events{ true };    /* App should continue processing input events */
		bool focused{ true };                 /* App is currently in focus at an operating system level */
		bool close_requested{ false };        /* Close requested */

	private:
		FPSCounter counter{};
	};
}