#pragma once

#include "input/controller.hpp"
#include "logger.hpp"

namespace sh {

bool
load_key_config(KeyMapping &o_p1, KeyMapping &o_p2, Logger *i_logger);

} // namespace sh
