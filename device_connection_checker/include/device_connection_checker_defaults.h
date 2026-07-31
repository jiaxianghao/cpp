/*
 * Copyright 2016-2025. UISEE TECHNOLOGIES (BEIJING) LTD. All rights reserved.
 * See LICENSE AGREEMENT file in the project root for full license information.
 */
#ifndef __DEVICE_CONNECTION_CHECKER_DEFAULTS_H__
#define __DEVICE_CONNECTION_CHECKER_DEFAULTS_H__

#include "device_connection_checker_interfaces.h"
#include <cstdio>
#include <sys/time.h>

/**
 * Default console logger implementation
 */
class ConsoleLogger : public ILogger
{
public:
    void log_error(const std::string& tag, const std::string& message) override {
        fprintf(stderr, "[ERROR][%s] %s\n", tag.c_str(), message.c_str());
    }
    
    void log_warning(const std::string& tag, const std::string& message) override {
        fprintf(stderr, "[WARN][%s] %s\n", tag.c_str(), message.c_str());
    }
    
    void log_info(const std::string& tag, const std::string& message) override {
        fprintf(stdout, "[INFO][%s] %s\n", tag.c_str(), message.c_str());
    }
};

/**
 * Default system time provider implementation
 */
class SystemTimeProvider : public ITimeProvider
{
public:
    float64_t get_current_time() const override {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }
};

#endif /* __DEVICE_CONNECTION_CHECKER_DEFAULTS_H__ */

