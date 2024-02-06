#include <engine/Application.h>

#include <logger/logger.h>
#include <taskflow/taskflow.hpp>

#include <plugin/plugin_manager.h>

using namespace engine;

int main()
{
    log_add_file_output("engine_log.log");
    log_add_cout_output();
    log_init("engine_app", "1.0.0");

    utl::plugin_system::uplugin_manager manager;
    auto& plugin = manager.load("D:\\GitHub\\new_renderer\\bin\\Debug\\engine\\glfw3.dll");

    auto app = stl::make_unique<CApplication>();
    app->create();
    app->loop();
    return 0;
}