#include <util/Logger.h>

#include <spdlog/cfg/env.h>

#define LOGGER_FORMAT "[%Y-%m-%d %H:%M:%S] [%^%l%$] %v"
#define PROJECT_NAME "NE"

namespace engine
{
	void LoggerInit()
	{
		auto env_val = spdlog::details::os::getenv("LOG_LEVEL");
		if (!env_val.empty())
		{
			spdlog::cfg::helpers::load_levels(env_val);
		}

#ifdef PLATFORM_ANDROID
		auto logger = spdlog::android_logger_mt(PROJECT_NAME, PROJECT_NAME);
#else
		auto logger = spdlog::stdout_color_mt(PROJECT_NAME);
#endif

		logger->set_pattern(LOGGER_FORMAT);
		logger->set_level(spdlog::level::trace);
		spdlog::set_default_logger(logger);
	}
}