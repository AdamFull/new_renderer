#include "Runtime.h"

// Filesystem headers
#include "../filesystem/VirtualFileSystemManager.h"
#include "../filesystem/native/NativeFileSystem.h"

#include "RuntimeContext.h"
#include <edge/util/Logger.h>

#include <edge/util/Profiler.h>

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace edge
{
	Runtime::Runtime(const RuntimeContext& context)
	{
	}

	int32_t Runtime::Initialize()
	{
		CreateWindow(window_properties);

		if (!window)
		{
			EDGE_LOGE("Window creation failed!");
			return ExitCode::FatalError;
		}

		// Initialize filesystem
		filesystem = std::make_unique<filesystem::VirtualFileSystemManager>();

		// Mount default engine root filesystem
		filesystem->Mount("/root", std::make_unique<filesystem::NativeFileSystem>("./"));
		filesystem->Mount("/source", std::make_unique<filesystem::NativeFileSystem>("./../../../"));

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
		EDGE_PROFILE_ZONE_SCOPED;

		//try
		{
			// Load the requested app
			if (AppRequested())
			{
				if (!StartApp())
				{
					throw std::runtime_error("Failed to load requested application");
				}

				// Compensate for load times of the app by rendering the first frame pre-emptively
				//counter.Frame();
				//app->Update(0.01667f);
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
		//catch (std::exception& e)
		//{
		//	EDGE_LOGE("Error Message: {}", e.what());
		//	EDGE_LOGE("Failed when running application {}", app->GetName());
		//
		//	OnAppError(app->GetName());
		//
		//	SetLastError(e.what());
		//	return 4;
		//}

		return 0;
	}

	void Runtime::Update()
	{
		EDGE_PROFILE_ZONE_SCOPED;

		auto delta_time = counter.GetFrameTime();

		auto new_windows_name = std::format("{} [{:.2f} fps; {:.2f} ms]", app->GetName(), counter.GetFps(), delta_time);
		//auto winname = std::format("Vulkan App {} fps ({:.2f} ms); {} gpu fps ({:.2f} ms); gpu memory ({:.2f}/{:.2f} Gb)", cpuCounter.fps(), cpuCounter.delta_time() * 1000.f, gpuCounter.fps(), gpuCounter.delta_time() * 1000.f, used_gpu_memory, total_gpu_memory);
		window->SetTitle(new_windows_name);

		if (focused || always_render)
		{
			//ui->NewFrame(delta_time);
			OnUpdate(delta_time);

			accumulated_frame_time += delta_time;
			while (accumulated_frame_time > fixed_delta_time)
			{
				app->FixedUpdate(fixed_delta_time);
				accumulated_frame_time -= fixed_delta_time;
			}

			app->Update(delta_time);
		}

		EDGE_PROFILE_FRAME_MARK;
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
		auto extent = Window::Extent{ std::max<uint32_t>(width, 420), std::max<uint32_t>(height, 320) };
		if ((window) && (width > 0) && (height > 0))
		{
			auto actual_extent = window->Resize(extent);

			if (app)
			{
				app->OnResize(actual_extent.width, actual_extent.height);
			}
		}
	}

	void Runtime::OnInputEvent(const InputEvent& input_event)
	{
		if (process_input_events && app)
		{
			app->OnInputEvent(input_event);
		}

		if (input_event.GetSource() == EventSource::Keyboard)
		{
			const auto& key_event = static_cast<const KeyInputEvent&>(input_event);

			if (key_event.GetCode() == KeyCode::Back || key_event.GetCode() == KeyCode::Escape)
			{
				Close();
			}
		}
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
			EDGE_LOGI("Closing App (Runtime: {:.1f})", execution_time);

			auto app_id = app->GetName();

			app->Finish();
		}

		app = app_ci->app_allocator();

		if (!app)
		{
			EDGE_LOGE("Failed to create a valid vulkan app.");
			return false;
		}

		app->SetName(app_ci->name);

		ApplicationOptions options{};
		options.benchmark_enabled = false;
		options.window = window.get();
		options.filesystem = filesystem.get();

		if (!app->Prepare(options))
		{
			EDGE_LOGE("Failed to prepare vulkan app.");
			return false;
		}

		OnAppStart(app_ci->name);

		return true;
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