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
load_single_bool(bool &o_val, const char *i_section, const char *i_key);

bool
save_single_bool(bool i_val, const char *i_section, const char *i_key);

bool
load_log_level(NHLogLevel &o_val);

bool
save_log_level(NHLogLevel i_val);

} // namespace sh
