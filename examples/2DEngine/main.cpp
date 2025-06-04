#include <edge/Edge.h>
#include "engine/App.h"

#include <edge/util/Logger.h>

int runtime_main(edge::IRuntimeContext& context) {
    auto runtime = edge::create_application_context(context);

    edge::ApplicationCreateInfo app_ci{};
    app_ci.name = "E2D";
    app_ci.app_allocator = []() { return std::make_unique<e2d::App>(); };

    runtime->request_application(&app_ci);

    auto code = runtime->initialize();
    if (code == edge::ExitCode::Success) {
        code = runtime->main_loop();
    }

    runtime->terminate(code);

    return 0;
}