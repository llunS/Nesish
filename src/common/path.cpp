#include "path.hpp"

namespace ln {

std::string
dirname(const std::string &i_path)
{
    if (i_path.empty())
    {
        return "";
    }
    auto last_slash_pos = i_path.rfind(LN_PATH_DELIMITER);
    if (last_slash_pos == std::string::npos)
    {
        return "";
    }
    // the root
    if (last_slash_pos == 0)
    {
        return LN_PATH_DELIMITER;
    }
    return i_path.substr(0, last_slash_pos);
}

std::string
join_exec_rel_path(const std::string &i_rel_path)
{
    auto exec_path = get_exec_path();
    if (exec_path.empty())
    {
        return "";
    }
    auto exec_dir = dirname(exec_path);
    if (exec_dir.empty())
    {
        return "";
    }
    return exec_dir + LN_PATH_DELIMITER + i_rel_path;
}

} // namespace ln
