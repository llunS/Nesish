#pragma once

#include <string>

#include "nhbase/dllexport.h"

namespace nb {

NB_API
std::string
dirname(const std::string &path);

NB_API
std::string
path_join(const std::string &lhs, const std::string &rhs);

NB_API
std::string
path_join_exe(const std::string &rel_path);

} // namespace nb
