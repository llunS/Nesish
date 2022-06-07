#include "path.hpp"

#include "common/path_private.hpp"

namespace ln {

std::string
dirname(const std::string &i_path)
{
    if (i_path.empty())
    {
        return "";
    }

    for (decltype(i_path.size()) i = 0; i < i_path.size(); ++i)
    {
        auto idx = i_path.size() - 1 - i;

        if (i_path[idx] == '/' || i_path[idx] == '\\')
        {
            // the root
            if (idx == 0)
            {
                return std::string(1, i_path[idx]);
            }
            else
            {
                return i_path.substr(0, idx);
            }
        }
    }
    return "";
}

std::string
path_join(const std::string &lhs, const std::string &rhs)
{
    return lhs + path_delimiter() + rhs;
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

    return path_join(exec_dir, i_rel_path);
}

} // namespace ln
