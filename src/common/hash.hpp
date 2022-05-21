#pragma once

#include <cstddef>

namespace ln {

struct EnumHash {
    template <typename T>
    std::size_t
    operator()(const T &t) const
    {
        return static_cast<std::size_t>(t);
    }
};

} // namespace ln
