#pragma once

#include "nesish/nesish.h"
#include "nhbase/klass.hpp"

#ifdef NDEBUG
#define SPDLOG_NO_SOURCE_LOC
#endif
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "nhbase/vc_intrinsics.hpp"
NB_VC_WARNING_PUSH
NB_VC_WARNING_DISABLE(6385)
#include "spdlog/spdlog.h"
NB_VC_WARNING_POP

#define SH_DEFAULT_LOG_LEVEL NH_LOG_INFO

#define SH_LOG_TRACE(i_logger, ...)                                            \
    SPDLOG_LOGGER_TRACE((i_logger)->logger, __VA_ARGS__)
#define SH_LOG_DEBUG(i_logger, ...)                                            \
    SPDLOG_LOGGER_DEBUG((i_logger)->logger, __VA_ARGS__)
#define SH_LOG_INFO(i_logger, ...)                                             \
    SPDLOG_LOGGER_INFO((i_logger)->logger, __VA_ARGS__)
#define SH_LOG_WARN(i_logger, ...)                                             \
    SPDLOG_LOGGER_WARN((i_logger)->logger, __VA_ARGS__)
#define SH_LOG_ERROR(i_logger, ...)                                            \
    SPDLOG_LOGGER_ERROR((i_logger)->logger, __VA_ARGS__)
#define SH_LOG_FATAL(i_logger, ...)                                            \
    SPDLOG_LOGGER_CRITICAL((i_logger)->logger, __VA_ARGS__)

namespace sh
{

const char *
log_level_to_name(NHLogLevel i_level);

struct Logger {
  public:
    Logger(NHLogLevel i_level);
    ~Logger();
    NB_KLZ_DELETE_COPY_MOVE(Logger);

  public:
    void
    set_level(NHLogLevel i_level);

  private:
    static void
    set_level(Logger *o_logger, NHLogLevel i_level);

  public:
    spdlog::logger *logger;
    NHLogLevel level;
};

} // namespace sh
