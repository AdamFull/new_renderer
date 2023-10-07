#include <engine/Application.h>

#include <libraries/utllib/logger/logger.h>

using namespace engine;

int main()
{
    log_add_file_output("engine_log.log");
    log_add_cout_output();
    log_init("engine_app", "1.0.0");

    std::unique_ptr<CApplication> app = std::make_unique<CApplication>();
    app->create();
    app->loop();
    return 0;
}