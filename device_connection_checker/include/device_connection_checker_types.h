/*
 * Copyright 2016-2025. UISEE TECHNOLOGIES (BEIJING) LTD. All rights reserved.
 * See LICENSE AGREEMENT file in the project root for full license information.
 */
#ifndef __DEVICE_CONNECTION_CHECKER_TYPES_H__
#define __DEVICE_CONNECTION_CHECKER_TYPES_H__

#include <cstdint>
#include <cmath>
#include <sys/time.h>
#include <cstdio>

/**
 * Basic type definitions
 */
typedef bool bool_t;
typedef int32_t int32_t;
typedef double float64_t;

#define UOS_TRUE true
#define UOS_FALSE false

/**
 * Log module definition
 */
#define UOS_MOD_OPERATION "OPERATION"

/**
 * Logging macros
 */
#define uos_log_e(module, fmt, ...) \
    fprintf(stderr, "[ERROR][%s] " fmt "\n", module, ##__VA_ARGS__)

#define uos_log_w(module, fmt, ...) \
    fprintf(stderr, "[WARN][%s] " fmt "\n", module, ##__VA_ARGS__)

#define uos_log_i(module, fmt, ...) \
    fprintf(stdout, "[INFO][%s] " fmt "\n", module, ##__VA_ARGS__)

/**
 * Time functions
 */
inline float64_t uos_getnow()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

/**
 * Math functions
 */
inline bool_t uos_fgreater(float64_t a, float64_t b)
{
    const float64_t epsilon = 1e-9;
    return (a - b) > epsilon;
}

inline bool_t uos_fless(float64_t a, float64_t b)
{
    const float64_t epsilon = 1e-9;
    return (b - a) > epsilon;
}

inline bool_t uos_fequal(float64_t a, float64_t b)
{
    const float64_t epsilon = 1e-9;
    return std::fabs(a - b) < epsilon;
}

#endif /* __DEVICE_CONNECTION_CHECKER_TYPES_H__ */

