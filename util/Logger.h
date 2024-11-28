#pragma once

#define LOGGER_FORMAT "[%Y-%m-%d %H:%M:%S] [%^%l%$] %v"

//#include <spdlog/fmt/std.h>
#include <spdlog/spdlog.h>

#ifdef PLATFORM_ANDROID
#include <spdlog/sinks/android_sink.h>
#else
#include <spdlog/sinks/stdout_color_sinks.h>
#endif

#define log_info(...) spdlog::info(__VA_ARGS__);
#define log_warning(...) spdlog::warn(__VA_ARGS__);
#define log_error(...) spdlog::error(__VA_ARGS__);
#define log_debug(...) spdlog::debug(__VA_ARGS__);