#include "dds_wrapper/Logger.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#endif

namespace dds_wrapper
{

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
    : log_level_(LogLevel::INFO)
    , console_output_(true)
    , file_output_(false)
    , initialized_(false)
{
}

Logger::~Logger()
{
    if (log_file_.is_open())
    {
        log_file_.close();
    }
}

void Logger::initialize(LogLevel level, bool console_output, bool file_output, const std::string& log_dir)
{
    std::lock_guard<std::mutex> lock(mutex_);

    log_level_ = level;
    console_output_ = console_output;
    file_output_ = file_output;
    log_dir_ = log_dir;

    if (file_output_)
    {
        // Create log directory if it doesn't exist
#ifdef _WIN32
        _mkdir(log_dir_.c_str());
#else
        mkdir(log_dir_.c_str(), 0755);
#endif

        // Open log file
        std::string log_filename = log_dir_ + "/dds_wrapper.log";
        log_file_.open(log_filename, std::ios::app);

        if (!log_file_.is_open())
        {
            std::cerr << "Failed to open log file: " << log_filename << std::endl;
            file_output_ = false;
        }
    }

    initialized_ = true;
}

void Logger::setLogLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(mutex_);
    log_level_ = level;
}

void Logger::log(LogLevel level, const std::string& message)
{
    if (level < log_level_)
    {
        return;
    }

    writeLog(level, message);
}

void Logger::debug(const std::string& message)
{
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message)
{
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message)
{
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message)
{
    log(LogLevel::ERROR, message);
}

std::string Logger::levelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNO";
    }
}

std::string Logger::getCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

void Logger::writeLog(LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::string timestamp = getCurrentTimestamp();
    std::string level_str = levelToString(level);
    std::string log_line = "[" + timestamp + "] [" + level_str + "] " + message;

    if (console_output_)
    {
        if (level == LogLevel::ERROR)
        {
            std::cerr << log_line << std::endl;
        }
        else
        {
            std::cout << log_line << std::endl;
        }
    }

    if (file_output_ && log_file_.is_open())
    {
        log_file_ << log_line << std::endl;
        log_file_.flush();
    }
}

} // namespace dds_wrapper
