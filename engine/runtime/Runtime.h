#pragma once

#include <engine/Application.h>
#include "Window.h"
#include <util/Performance.h>
#include "ui/EngineUI.h"

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace engine
{
	class RuntimeContext;

	enum ExitCode
	{
		Success = 0, /* App executed as expected */
		Close,       /* App has been requested to close at initialization */
		FatalError   /* App encountered an unexpected error */
	};

	class Runtime
	{
	public:
		Runtime(const RuntimeContext& context);

		virtual ~Runtime() = default;

		/**
		 * @brief Initialize the platform
		 * @return An exit code representing the outcome of initialization
		 */
		virtual int32_t Initialize();

		/**
		 * @brief Handles the main update and render loop
		 * @return An exit code representing the outcome of the loop
		 */
		int32_t MainLoop();

		/**
		 * @brief Handles the update and render of a frame.
		 * Called either from main_loop(), or from a platform-specific
		 * frame-looping mechanism, typically tied to platform screen refeshes.
		 * @return An exit code representing the outcome of the loop
		 */
		int32_t MainLoopFrame();

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

		Window& GetWindow();

		Application& GetApp() const;
		Application& GetApp();

		void SetLastError(const std::string& error);

		void SetFocus(bool focused);

		void RequestApplication(ApplicationCreateInfo* app_ci);
		bool AppRequested();
		bool StartApp();

		void ForceSimulationFps(float fps);

		// Force the application to always render even if it is not in focus
		void ForceRender(bool should_always_render);

		void DisableInputProcessing();

		void SetWindowProperties(const Window::OptionalProperties& properties);
	protected:
		std::unique_ptr<Window> window{ nullptr };
		std::unique_ptr<filesystem::IVirtualFileSystemInterface> virtual_filesystem{};

		std::unique_ptr<Application> app{ nullptr };

		std::unique_ptr<EngineUI> ui{};

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

		Window::Properties window_properties; /* Source of truth for window state */
		bool fixed_simulation_fps{ false };   /* Delta time should be fixed with a fabricated value */
		bool always_render{ false };          /* App should always render even if not in focus */
		float simulation_frame_time = 0.016f; /* A fabricated delta time */
		bool process_input_events{ true };    /* App should continue processing input events */
		bool focused{ true };                 /* App is currently in focus at an operating system level */
		bool close_requested{ false };        /* Close requested */

	private:
		FPSCounter counter{};
	};
}