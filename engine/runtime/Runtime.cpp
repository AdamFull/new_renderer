#include <engine/runtime/Runtime.h>

#include <engine/runtime/RuntimeContext.h>
#include <util/Logger.h>

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace engine
{
	Runtime::Runtime(const RuntimeContext& context)
	{

	}

	int32_t Runtime::Initialize()
	{
		CreateWindow(window_properties);

		if (!window)
		{
			log_error("Window creation failed!");
			return ExitCode::FatalError;
		}

		return 0;
	}

	int32_t Runtime::MainLoop()
	{
		int32_t exit_code = ExitCode::Success;
		while ((exit_code == ExitCode::Success) && !window->ShouldClose() && !close_requested)
		{
			exit_code = MainLoopFrame();
		}

		return exit_code;
	}

	int32_t Runtime::MainLoopFrame()
	{
		try
		{
			// Load the requested app
			if (AppRequested())
			{
				if (!StartApp())
				{
					throw std::runtime_error("Failed to load requested application");
				}

				// Compensate for load times of the app by rendering the first frame pre-emptively
				counter.Frame();
				app->Update(0.01667f);
			}

			if (!app)
			{
				return -1;
			}

			Update();

			if (app->ShouldClose())
			{
				std::string id = app->GetName();
				OnAppClose(id);
				app->Finish();
			}

			window->ProcessEvents();

			counter.Frame();
		}
		catch (std::exception& e)
		{
			log_error("Error Message: {}", e.what());
			log_error("Failed when running application {}", app->GetName());

			OnAppError(app->GetName());

			SetLastError(e.what());
			return 4;
		}

		return 0;
	}

	void Runtime::Update()
	{
		auto delta_time = counter.GetFrameTime();

		if (focused || always_render)
		{
			OnUpdate(delta_time);

			if (fixed_simulation_fps)
			{
				delta_time = simulation_frame_time;
			}

			//app->update_overlay(delta_time, [=]() {
			//	on_update_ui_overlay(*active_app->get_drawer());
			//	});
			app->Update(delta_time);

			// TODO: This original code is bad. May be think about something better...
			//if (auto* app = dynamic_cast<RenderApplication*>(app.get()))
			//{
			//	//if (app->HasRenderContext())
			//	//{
			//	//	OnPostDraw(reinterpret_cast<RenderContext&>(app->GetRenderContext()));
			//	//}
			//}
		}
	}

	void Runtime::Terminate(int32_t code)
	{
		if (app)
		{
			std::string id = app->GetName();
			OnAppClose(id);
			app->Finish();
		}

		app.reset();
		window.reset();

		OnClose();
	}

	void Runtime::Close()
	{
		if (window)
		{
			window->Close();
		}

		// Fallback incase a window is not yet in use
		close_requested = true;
	}

	void Runtime::OnResize(uint32_t width, uint32_t height)
	{

	}

	void Runtime::OnInputEvent(const InputEvent& input_event)
	{

	}

	Window& Runtime::GetWindow()
	{
		return *window;
	}

	Application& Runtime::GetApp() const
	{
		return *app;
	}

	Application& Runtime::GetApp()
	{
		return *app;
	}

	void Runtime::SetLastError(const std::string& error)
	{

	}

	void Runtime::SetFocus(bool _focused)
	{
		focused = _focused;
	}

	void Runtime::RequestApplication(ApplicationCreateInfo* app_ci)
	{
		app_create_info = app_ci;
	}

	bool Runtime::AppRequested()
	{
		return app_create_info != nullptr;
	}

	bool Runtime::StartApp()
	{
		auto* app_ci = app_create_info;
		app_create_info = nullptr;

		if (app)
		{
			auto execution_time = counter.ElapsedSeconds();
			log_info("Closing App (Runtime: {:.1f})", execution_time);

			auto app_id = app->GetName();

			app->Finish();
		}

		// Reset target environment to default prior to each sample to properly support batch mode
		//vkb::GLSLCompiler::reset_target_environment();

		app = app_ci->app_allocator();

		if (!app)
		{
			log_error("Failed to create a valid vulkan app.");
			return false;
		}

		app->SetName(app_ci->name);

		if (!app->Prepare({ false, window.get() }))
		{
			log_error("Failed to prepare vulkan app.");
			return false;
		}

		OnAppStart(app_ci->name);

		return true;
	}

	void Runtime::ForceSimulationFps(float fps)
	{
		fixed_simulation_fps = true;
		simulation_frame_time = 1 / fps;
	}

	void Runtime::ForceRender(bool should_always_render)
	{
		always_render = should_always_render;
	}

	void Runtime::DisableInputProcessing()
	{
		process_input_events = false;
	}

	void Runtime::SetWindowProperties(const Window::OptionalProperties& properties)
	{
		window_properties.title = properties.title.has_value() ? properties.title.value() : window_properties.title;
		window_properties.mode = properties.mode.has_value() ? properties.mode.value() : window_properties.mode;
		window_properties.resizable = properties.resizable.has_value() ? properties.resizable.value() : window_properties.resizable;
		window_properties.vsync = properties.vsync.has_value() ? properties.vsync.value() : window_properties.vsync;
		window_properties.extent.width = properties.extent.width.has_value() ? properties.extent.width.value() : window_properties.extent.width;
		window_properties.extent.height = properties.extent.height.has_value() ? properties.extent.height.value() : window_properties.extent.height;
	}

	void Runtime::OnUpdate(float delta_time)
	{

	}

	void Runtime::OnAppError(const std::string& app_id)
	{

	}

	void Runtime::OnAppStart(const std::string& app_id)
	{

	}

	void Runtime::OnAppClose(const std::string& app_id)
	{

	}

	void Runtime::OnClose()
	{

	}
}