#ifndef LN_COMMON_FILESYSTEM_HPP
#define LN_COMMON_FILESYSTEM_HPP

#include <string>

namespace ln {

bool
file_exists(const std::string &i_path);

bool
file_rename(const std::string &i_old, const std::string &i_new);

} // namespace ln

#endif // LN_COMMON_FILESYSTEM_HPP
