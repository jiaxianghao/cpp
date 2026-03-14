#ifndef DDS_WRAPPER_LOGGER_H
#define DDS_WRAPPER_LOGGER_H

#include "Types.h"
#include <string>
#include <fstream>
#include <mutex>
#include <memory>

namespace dds_wrapper
{

// Singleton logger class
class Logger
{
public:
    static Logger& getInstance();

    void initialize(LogLevel level, bool console_output, bool file_output, const std::string& log_dir);
    void setLogLevel(LogLevel level);
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();

    std::string levelToString(LogLevel level);
    std::string getCurrentTimestamp();
    void writeLog(LogLevel level, const std::string& message);

    LogLevel log_level_;
    bool console_output_;
    bool file_output_;
    std::string log_dir_;
    std::ofstream log_file_;
    std::mutex mutex_;
    bool initialized_;
};

} // namespace dds_wrapper

#endif // DDS_WRAPPER_LOGGER_H
