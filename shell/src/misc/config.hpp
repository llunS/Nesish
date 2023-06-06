#pragma once

#include "input/controller.hpp"
#include "logger.hpp"

namespace sh {

bool
load_key_config(KeyMapping &o_p1, KeyMapping &o_p2, Logger *i_logger);

bool
save_key_config(NHCtrlPort i_port, NHKey i_key, VirtualKey i_vkey,
                Logger *i_logger);

bool
reset_default_key_config(KeyMapping *o_p1, KeyMapping *o_p2, Logger *i_logger);

} // namespace sh
