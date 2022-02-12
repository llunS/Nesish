#ifndef LN_COMMON_LOGGER_HPP
#define LN_COMMON_LOGGER_HPP

#include "spdlog/spdlog.h"

namespace ln {

void
init_logger(spdlog::level::level_enum i_level);

spdlog::logger *
get_logger();

} // namespace ln

#endif // LN_COMMON_LOGGER_HPP
