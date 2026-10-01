#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

inline void SetupLogger(spdlog::level::level_enum level = spdlog::level::trace) {
    auto default_logger = spdlog::default_logger();

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::ansicolor_stdout_sink_st>());

    // create logger
    const auto *logger_name = "logger";
    auto combined_logger = std::make_shared<spdlog::logger>(logger_name, begin(sinks), end(sinks));
    const auto *pattern = "%Y-%m-%d %H:%M:%S.%f [%t] %^[%8l]%$ [%s:%#] [%!] %v";
    combined_logger->set_pattern(pattern);
    combined_logger->set_level(level);

    register_logger(combined_logger);
    set_default_logger(combined_logger);

    spdlog::flush_on(spdlog::level::trace);
}

#endif // TEST_COMMON_H
