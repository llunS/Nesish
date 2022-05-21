#pragma once

#include "spdlog/spdlog.h"

#include "common/dllexport.h"

#define LN_DEFAULT_LOG_LEVEL spdlog::level::info

namespace ln {

LN_COMMON_API
void
init_logger(spdlog::level::level_enum i_level = LN_DEFAULT_LOG_LEVEL);

LN_COMMON_API
spdlog::logger *
get_logger();

} // namespace ln
