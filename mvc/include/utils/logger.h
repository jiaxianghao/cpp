#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>

// Logger wrapper class for easy logging across the application
class Logger
{
public:
    // Initialize the logger system
    static void initialize(const std::string& app_name = "DatabaseApp",
                          const std::string& log_file = "logs/app.log",
                          size_t max_file_size = 1024 * 1024 * 5,  // 5MB
                          size_t max_files = 3,
                          spdlog::level::level_enum level = spdlog::level::info);

    // Set log level
    static void setLevel(spdlog::level::level_enum level);

    // Logging methods
    template<typename... Args>
    static void trace(const std::string& fmt, Args&&... args)
    {
        getInstance()->trace(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void debug(const std::string& fmt, Args&&... args)
    {
        getInstance()->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info(const std::string& fmt, Args&&... args)
    {
        getInstance()->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warn(const std::string& fmt, Args&&... args)
    {
        getInstance()->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(const std::string& fmt, Args&&... args)
    {
        getInstance()->error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void critical(const std::string& fmt, Args&&... args)
    {
        getInstance()->critical(fmt, std::forward<Args>(args)...);
    }

    // Flush logs to disk
    static void flush();

    // Shutdown logger
    static void shutdown();

private:
    static std::shared_ptr<spdlog::logger> getInstance();
    static std::shared_ptr<spdlog::logger> logger_;
};

#endif // LOGGER_H
