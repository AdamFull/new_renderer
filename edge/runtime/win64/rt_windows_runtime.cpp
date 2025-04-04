#include "rt_windows_runtime.h"

#include "../common/rt_glfw_window.h"
#include "../rt_headless_wIndow.h"
#include "../../fs/fs_helper.h"

#include "rt_windows_runtime_context.h"

namespace edge::win64
{
	WindowsRuntime::WindowsRuntime(const IRuntimeContext& context) :
		Runtime(context) {

	}

    int32_t WindowsRuntime::initialize() {
        char executable_path[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, executable_path, MAX_PATH);

        auto exec_path = std::filesystem::weakly_canonical(executable_path);

        std::array<char, 512> buffer{};
        internal_data_path = exec_path.parent_path().string();
        external_data_path = (std::filesystem::temp_directory_path() / exec_path.stem()).string();
        if (!std::filesystem::exists(external_data_path)) {
            std::filesystem::create_directories(external_data_path);
        }

        internal_resources_path = fs::path_append(internal_data_path, "internal.pak", buffer);
        external_resources_path = fs::path_append(external_data_path, "external.pak", buffer);

        auto code = Runtime::initialize();
        if (code != ExitCode::Success) {
            return code;
        }

        return ExitCode::Success;
    }

	void WindowsRuntime::create_window(const Window::Properties& properties) {
		if (properties.mode == Window::Mode::Headless) {
			window = std::make_unique<HeadlessWindow>(properties);
		}
		else {
			window = std::make_unique<GlfwWindow>(this, properties);
		}
	}
}