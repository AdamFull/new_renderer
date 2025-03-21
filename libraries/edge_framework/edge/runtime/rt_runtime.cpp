#include "rt_runtime.h"

// Filesystem headers
#include "../fs/fs_virtual_filesystem.h"
#include "../fs/native/fs_native_filesystem.h"
#include "../fs/zip/fs_zip_filesystem.h"

#include "rt_runtime_context.h"
#include "../util/Logger.h"

#include "../util/Profiler.h"

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace edge {
	Runtime::Runtime(const IRuntimeContext& context) {
	}

	auto Runtime::initialize() -> int32_t {
		create_window(window_properties);

		if (!window) {
			EDGE_LOGE("Window creation failed!");
			return ExitCode::FatalError;
		}

		// Initialize filesystem
		filesystem = std::make_unique<fs::VirtualFilesystem>();

		// Mount default engine root filesystem
		filesystem->mount("/root", new fs::NativeFilesystem("./"));
		filesystem->mount("/source", new fs::NativeFilesystem("./../../../"));
		filesystem->mount("/resources", new fs::ZipFilesystem("resources.zip"));

		return 0;
	}

	auto Runtime::main_loop() -> int32_t {
		int32_t exit_code = ExitCode::Success;
		while ((exit_code == ExitCode::Success) && !window->should_close() && !close_requested) {
			exit_code = main_loop_frame();
		}

		return exit_code;
	}

	auto Runtime::main_loop_frame() -> int32_t {
		EDGE_PROFILE_ZONE_SCOPED;

		//try
		{
			// Load the requested app
			if (app_requested()) {
				if (!start_app()) {
					EDGE_LOGE("[Runtime] Failed to load requested application");
				}

				// Compensate for load times of the app by rendering the first frame pre-emptively
				//counter.Frame();
				//app->Update(0.01667f);
			}

			if (!app) {
				return -1;
			}

			update();

			if (app->should_close()) {
				std::string id = app->get_name();
				on_app_close(id);
				app->finish();
			}

			window->process_events();

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

	void Runtime::update() {
		EDGE_PROFILE_ZONE_SCOPED;

		auto delta_time = counter.GetFrameTime();

		auto new_windows_name = std::format("{} [{:.2f} fps; {:.2f} ms]", app->get_name(), counter.GetFps(), delta_time);
		//auto winname = std::format("Vulkan App {} fps ({:.2f} ms); {} gpu fps ({:.2f} ms); gpu memory ({:.2f}/{:.2f} Gb)", cpuCounter.fps(), cpuCounter.delta_time() * 1000.f, gpuCounter.fps(), gpuCounter.delta_time() * 1000.f, used_gpu_memory, total_gpu_memory);
		window->set_title(new_windows_name);

		if (focused || always_render) {
			//ui->NewFrame(delta_time);
			on_update(delta_time);

			accumulated_frame_time += delta_time;
			while (accumulated_frame_time > fixed_delta_time) {
				app->fixed_update(fixed_delta_time);
				accumulated_frame_time -= fixed_delta_time;
			}

			app->update(delta_time);
		}

		EDGE_PROFILE_FRAME_MARK;
	}

	void Runtime::terminate(int32_t code) {
		if (app) {
			std::string id = app->get_name();
			on_app_close(id);
			app->finish();
		}

		app.reset();
		window.reset();

		on_close();
	}

	void Runtime::close() {
		if (window) {
			window->close();
		}

		// Fallback incase a window is not yet in use
		close_requested = true;
	}

	void Runtime::on_resize(uint32_t width, uint32_t height) {
		auto extent = Window::Extent{ std::max<uint32_t>(width, 420), std::max<uint32_t>(height, 320) };
		if ((window) && (width > 0) && (height > 0)) {
			auto actual_extent = window->resize(extent);

			if (app) {
				app->on_resize(actual_extent.width, actual_extent.height);
			}
		}
	}

	void Runtime::on_input_event(const InputEvent& input_event) {
		if (process_input_events && app) {
			app->on_input_event(input_event);
		}

		if (input_event.get_source() == EventSource::Keyboard) {
			const auto& key_event = static_cast<const KeyInputEvent&>(input_event);

			if (key_event.get_code() == KeyCode::Back || key_event.get_code() == KeyCode::Escape) {
				close();
			}
		}
	}

	void Runtime::set_last_error(const std::string& error) {

	}

	auto Runtime::start_app() -> bool {
		auto* app_ci = app_create_info;
		app_create_info = nullptr;

		if (app) {
			auto execution_time = counter.ElapsedSeconds();
			EDGE_LOGI("[Runtime] Closing App (Runtime: {:.1f})", execution_time);

			auto app_id = app->get_name();

			app->finish();
		}

		app = app_ci->app_allocator();

		if (!app) {
			EDGE_LOGE("[Runtime] Failed to create a valid app.");
			return false;
		}

		app->set_name(app_ci->name);

		ApplicationOptions options{};
		options.benchmark_enabled = false;
		options.window = window.get();
		options.filesystem = filesystem.get();

		if (!app->prepare(options)) {
			EDGE_LOGE("[Runtime] Failed to prepare app.");
			return false;
		}

		on_app_start(app_ci->name);

		return true;
	}

	void Runtime::set_window_properties(const Window::OptionalProperties& properties) {
		window_properties.title = properties.title.has_value() ? properties.title.value() : window_properties.title;
		window_properties.mode = properties.mode.has_value() ? properties.mode.value() : window_properties.mode;
		window_properties.resizable = properties.resizable.has_value() ? properties.resizable.value() : window_properties.resizable;
		window_properties.vsync = properties.vsync.has_value() ? properties.vsync.value() : window_properties.vsync;
		window_properties.extent.width = properties.extent.width.has_value() ? properties.extent.width.value() : window_properties.extent.width;
		window_properties.extent.height = properties.extent.height.has_value() ? properties.extent.height.value() : window_properties.extent.height;
	}

	void Runtime::on_update(float delta_time) {

	}

	void Runtime::on_app_error(const std::string& app_id) {

	}

	void Runtime::on_app_start(const std::string& app_id) {

	}

	void Runtime::on_app_close(const std::string& app_id) {

	}

	void Runtime::on_close()
	{

	}
}