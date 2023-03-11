#pragma once

#include "nesish/nesish.h"

#include "fmt/core.h"

#define NH_LOG(i_logger, i_level, ...)                                         \
    if ((i_logger) && (i_logger)->active >= (i_level))                         \
    {                                                                          \
        std::string s = fmt::format(__VA_ARGS__);                              \
        (i_logger)->log((i_level), s.c_str(), (i_logger)->user);               \
    }
#define NH_LOG_FATAL(i_logger, ...) NH_LOG(i_logger, NH_LOG_FATAL, __VA_ARGS__)
#define NH_LOG_ERROR(i_logger, ...) NH_LOG(i_logger, NH_LOG_ERROR, __VA_ARGS__)
#define NH_LOG_WARN(i_logger, ...) NH_LOG(i_logger, NH_LOG_WARN, __VA_ARGS__)
#define NH_LOG_INFO(i_logger, ...) NH_LOG(i_logger, NH_LOG_INFO, __VA_ARGS__)
#define NH_LOG_DEBUG(i_logger, ...) NH_LOG(i_logger, NH_LOG_DEBUG, __VA_ARGS__)
#define NH_LOG_TRACE(i_logger, ...) NH_LOG(i_logger, NH_LOG_TRACE, __VA_ARGS__)
