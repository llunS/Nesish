#include "nhbase/path.hpp"

#include "path_private.hpp"

namespace nb {

std::string
dirname(const std::string &path)
{
    if (path.empty())
    {
        return "";
    }

    for (decltype(path.size()) i = 0; i < path.size(); ++i)
    {
        auto idx = path.size() - 1 - i;

        if (path[idx] == '/' || path[idx] == '\\')
        {
            // the root
            if (idx == 0)
            {
                return std::string(1, path[idx]);
            }
            else
            {
                return path.substr(0, idx);
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
path_join_exe(const std::string &rel_path)
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

    return path_join(exec_dir, rel_path);
}

} // namespace nb
