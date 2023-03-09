#pragma once

#include <string>

#include "common/dllexport.h"

namespace nh {

LN_COMMON_API
bool
file_exists(const std::string &i_path);

LN_COMMON_API
bool
file_rename(const std::string &i_old, const std::string &i_new, bool i_force);

} // namespace nh
