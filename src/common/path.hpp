#pragma once

#include <string>

#include "common/dllexport.h"

#define LN_PATH_DELIMITER "/"

namespace ln {

LN_COMMON_API
std::string
path_native(const std::string &i_path);

LN_COMMON_API
std::string
dirname(const std::string &i_path);

LN_COMMON_API
std::string
join_exec_rel_path(const std::string &i_rel_path);

LN_COMMON_API
std::string
get_exec_path();

} // namespace ln
