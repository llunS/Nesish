#pragma once

#include <string>

#include "nhbase/dllexport.h"

namespace nb {

NB_API
bool
file_exists(const std::string &path);

NB_API
bool
file_copy(const std::string &from, const std::string &to);

NB_API
bool
file_rename(const std::string &from, const std::string &to, bool force);

} // namespace nb
