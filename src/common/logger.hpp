#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

#include "common/dllexport.h"

#define LN_DEFAULT_LOG_LEVEL spdlog::level::info

namespace nh {

LN_COMMON_API
void
init_logger(spdlog::level::level_enum i_level = LN_DEFAULT_LOG_LEVEL);

LN_COMMON_API
spdlog::logger *
get_logger();

} // namespace nh

template <typename... Args>
void
LN_LOG_ARGS_UNUSED(Args &&...)
{
}

#define LN_LOG_TRACE(i_logger, ...)                                            \
    LN_LOG_ARGS_UNUSED(i_logger, __VA_ARGS__);                                 \
    SPDLOG_LOGGER_TRACE(i_logger, __VA_ARGS__)
#define LN_LOG_DEBUG(i_logger, ...)                                            \
    LN_LOG_ARGS_UNUSED(i_logger, __VA_ARGS__);                                 \
    SPDLOG_LOGGER_DEBUG(i_logger, __VA_ARGS__)
#define LN_LOG_INFO(i_logger, ...)                                             \
    LN_LOG_ARGS_UNUSED(i_logger, __VA_ARGS__);                                 \
    SPDLOG_LOGGER_INFO(i_logger, __VA_ARGS__)
#define LN_LOG_WARN(i_logger, ...)                                             \
    LN_LOG_ARGS_UNUSED(i_logger, __VA_ARGS__);                                 \
    SPDLOG_LOGGER_WARN(i_logger, __VA_ARGS__)
#define LN_LOG_ERROR(i_logger, ...)                                            \
    LN_LOG_ARGS_UNUSED(i_logger, __VA_ARGS__);                                 \
    SPDLOG_LOGGER_ERROR(i_logger, __VA_ARGS__)
#define LN_LOG_FATAL(i_logger, ...)                                            \
    LN_LOG_ARGS_UNUSED(i_logger, __VA_ARGS__);                                 \
    SPDLOG_LOGGER_CRITICAL(i_logger, __VA_ARGS__)
