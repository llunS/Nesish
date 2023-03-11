#pragma once

#include "nesish/nesish.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

#define SH_DEFAULT_LOG_LEVEL NH_LOG_INFO

#define SH_LOG_TRACE(i_logger, ...) SPDLOG_LOGGER_TRACE(i_logger, __VA_ARGS__)
#define SH_LOG_DEBUG(i_logger, ...) SPDLOG_LOGGER_DEBUG(i_logger, __VA_ARGS__)
#define SH_LOG_INFO(i_logger, ...) SPDLOG_LOGGER_INFO(i_logger, __VA_ARGS__)
#define SH_LOG_WARN(i_logger, ...) SPDLOG_LOGGER_WARN(i_logger, __VA_ARGS__)
#define SH_LOG_ERROR(i_logger, ...) SPDLOG_LOGGER_ERROR(i_logger, __VA_ARGS__)
#define SH_LOG_FATAL(i_logger, ...)                                            \
    SPDLOG_LOGGER_CRITICAL(i_logger, __VA_ARGS__)

namespace sh {

void
init_logger(NHLogLevel i_level);

spdlog::logger *
get_logger();

NHLogLevel
get_log_level();

} // namespace sh
