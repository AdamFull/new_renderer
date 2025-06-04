#include <edge/Edge.h>
#include "engine/EngineApplication.h"

#include <edge/util/Logger.h>

#include <iostream>

auto runtime_main(edge::IRuntimeContext& context) -> int {
    auto runtime = edge::create_application_context(context);

    edge::ApplicationCreateInfo app_ci{};
    app_ci.features.use_graphics = true;
    app_ci.features.use_uploader = true;
    app_ci.name = "MyApp";
    app_ci.app_allocator = []() { return std::make_unique<engine::EngineApplication>(); };
    
    runtime->request_application(&app_ci);
    
    auto code = runtime->initialize();
    if (code == edge::ExitCode::Success) {
    	code = runtime->main_loop();
    }
    
    runtime->terminate(code);

    return 0;
}