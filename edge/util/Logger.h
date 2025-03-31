#pragma once

#define LOGGER_FORMAT "[%Y-%m-%d %H:%M:%S] [%^%l%$] %v"

#include "debugbreak.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#ifdef PLATFORM_ANDROID
#include <spdlog/sinks/android_sink.h>
#else
#include <spdlog/sinks/stdout_color_sinks.h>
#endif

#define EDGE_LOGI(...) spdlog::info(__VA_ARGS__);
#define EDGE_LOGW(...) spdlog::warn(__VA_ARGS__);
#define EDGE_LOGE(...) spdlog::error(__VA_ARGS__); debugbreak();
#define EDGE_LOGD(...) spdlog::debug(__VA_ARGS__);
#define EDGE_LOGT(...) spdlog::trace(__VA_ARGS__);

namespace edge
{
	struct LoggerCreateInfo
	{
		std::string_view logger_name{ "logger" };
		std::string_view output_log_path{ "output.log" };
	};

	inline void CreateLogger(const LoggerCreateInfo& create_info)
	{
#ifdef PLATFORM_ANDROID
		auto console_sink = std::make_shared<spdlog::sinks::android_sink_mt>(create_info.logger_name.data());
		spdlog::sinks_init_list sink_list = { console_sink };
#else
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(create_info.output_log_path.data(), true);
		spdlog::sinks_init_list sink_list = { file_sink };
#endif

#ifdef PLATFORM_ANDROID
		auto logger = spdlog::android_logger_mt(create_info.logger_name.data(), create_info.logger_name.data());
#else
		auto logger = std::make_shared<spdlog::logger>(create_info.logger_name.data(), sink_list.begin(), sink_list.end());
#endif

#ifdef NDEBUG
		spdlog::set_level(spdlog::level::info);
#ifndef PLATFORM_ANDROID
		console_sink->set_level(spdlog::level::info);
#endif
		file_sink->set_level(spdlog::level::info);
		spdlog::info("Logger initialized in RELEASE mode");
#else
		logger->set_level(spdlog::level::trace);
#ifdef PLATFORM_ANDROID
		console_sink->set_level(spdlog::level::trace);
#else
        file_sink->set_level(spdlog::level::trace);
#endif
		spdlog::debug("Logger initialized in DEBUG mode");
#endif

		logger->set_pattern(LOGGER_FORMAT);

		//spdlog::register_logger(logger);
		spdlog::set_default_logger(logger);
	}
}