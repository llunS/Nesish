#ifndef LN_COMMON_PATH_HPP
#define LN_COMMON_PATH_HPP

#include <string>

#define LN_PATH_DELIMITER "/"

namespace ln {

std::string
path_native(const std::string &i_path);

std::string
dirname(const std::string &i_path);

std::string
join_exec_rel_path(const std::string &i_rel_path);

std::string
get_exec_path();

} // namespace ln

#endif // LN_COMMON_PATH_HPP
