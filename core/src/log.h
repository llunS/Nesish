#pragma once

#include "nesish/nesish.h"

#include <stdio.h>

#define LOG_(logger, level, ...)                                               \
    if ((logger) && (logger)->active >= (level)) {                             \
        char msg[64]; /* 64 should be enough */                                \
        (void)snprintf(msg, sizeof(msg), __VA_ARGS__);                         \
        (logger)->log((level), msg, (logger)->user);                           \
    }
#define LOG_FATAL(logger, ...) LOG_(logger, NH_LOG_FATAL, __VA_ARGS__)
#define LOG_ERROR(logger, ...) LOG_(logger, NH_LOG_ERROR, __VA_ARGS__)
#define LOG_WARN(logger, ...) LOG_(logger, NH_LOG_WARN, __VA_ARGS__)
#define LOG_INFO(logger, ...) LOG_(logger, NH_LOG_INFO, __VA_ARGS__)
#define LOG_DEBUG(logger, ...) LOG_(logger, NH_LOG_DEBUG, __VA_ARGS__)
#define LOG_TRACE(logger, ...) LOG_(logger, NH_LOG_TRACE, __VA_ARGS__)
