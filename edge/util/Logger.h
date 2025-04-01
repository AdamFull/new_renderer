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

#ifdef NDEBUG
#define EDGE_LOG_LEVEL spdlog::level::info
#else
#define EDGE_LOG_LEVEL spdlog::level::trace
#endif

namespace edge
{
	struct LoggerCreateInfo
	{
		std::string_view logger_name{ "logger" };
		std::string_view output_log_path{ "output.log" };
	};

	inline void create_default_logger(std::string_view logger_name, std::string_view output_path) {
		spdlog::sinks_init_list sink_list{
#ifdef PLATFORM_ANDROID
			std::make_shared<spdlog::sinks::android_sink_mt>(logger_name.data())
#else
			std::make_shared<spdlog::sinks::basic_file_sink_mt>(output_path.data(), true),
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
#endif
		};

		for (auto& sink : sink_list) {
			sink->set_level(EDGE_LOG_LEVEL);
		}

		auto logger = std::make_shared<spdlog::logger>(logger_name.data(), sink_list.begin(), sink_list.end());
		logger->set_level(EDGE_LOG_LEVEL);
		logger->set_pattern(LOGGER_FORMAT);
		spdlog::set_default_logger(logger);
	}

	inline void CreateLogger(const LoggerCreateInfo& create_info)
	{
		create_default_logger(create_info.logger_name, create_info.output_log_path);
	}
}