#include "utils/logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <vector>
#include <iostream>
#include <filesystem>

std::shared_ptr<spdlog::logger> Logger::logger_ = nullptr;

void Logger::initialize(const std::string& app_name,
                       const std::string& log_file,
                       size_t max_file_size,
                       size_t max_files,
                       spdlog::level::level_enum level)
{
    try
    {
        // Create logs directory if it doesn't exist
        std::filesystem::path log_path(log_file);
        if (log_path.has_parent_path())
        {
            std::filesystem::create_directories(log_path.parent_path());
        }

        // Create sinks
        std::vector<spdlog::sink_ptr> sinks;

        // Console sink with colors
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(level);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
        sinks.push_back(console_sink);

        // Rotating file sink
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_file, max_file_size, max_files);
        file_sink->set_level(spdlog::level::trace);  // Log everything to file
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] [%t] %v");
        sinks.push_back(file_sink);

        // Create logger with multiple sinks
        logger_ = std::make_shared<spdlog::logger>(app_name, sinks.begin(), sinks.end());
        logger_->set_level(level);
        logger_->flush_on(spdlog::level::warn);  // Auto flush on warnings and errors

        // Register as default logger
        spdlog::register_logger(logger_);
        spdlog::set_default_logger(logger_);

        logger_->info("Logger initialized: {}", app_name);
        logger_->info("Log file: {}", log_file);
        logger_->info("Log level: {}", spdlog::level::to_string_view(level));
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        throw;
    }
}

void Logger::setLevel(spdlog::level::level_enum level)
{
    if (logger_)
    {
        logger_->set_level(level);
        logger_->info("Log level changed to: {}", spdlog::level::to_string_view(level));
    }
}

std::shared_ptr<spdlog::logger> Logger::getInstance()
{
    if (!logger_)
    {
        // Create a default console logger if not initialized
        logger_ = spdlog::stdout_color_mt("default");
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        logger_->warn("Logger not initialized, using default console logger");
    }
    return logger_;
}

void Logger::flush()
{
    if (logger_)
    {
        logger_->flush();
    }
}

void Logger::shutdown()
{
    if (logger_)
    {
        logger_->info("Logger shutting down");
        logger_->flush();
        spdlog::shutdown();
        logger_ = nullptr;
    }
}
