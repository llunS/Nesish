#pragma once

#include <string>

#include "common/dllexport.h"

namespace ln {

LN_COMMON_API
std::string
dirname(const std::string &i_path);

LN_COMMON_API
std::string
path_join(const std::string &lhs, const std::string &rhs);

LN_COMMON_API
std::string
join_exec_rel_path(const std::string &i_rel_path);

} // namespace ln
