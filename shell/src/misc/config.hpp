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

bool
load_sleepless(bool &o_val);

bool
save_sleepless(bool i_val);

bool
load_log_level(NHLogLevel &o_val);

bool
save_log_level(NHLogLevel i_val);

} // namespace sh
