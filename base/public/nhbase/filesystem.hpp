#pragma once

#include <string>

#include "nhbase/dllexport.h"

namespace nb {

NB_API
bool
file_exists(const std::string &path);

NB_API
bool
file_rename(const std::string &path_old, const std::string &path_new,
            bool force);

} // namespace nb
