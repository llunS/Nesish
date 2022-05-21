#pragma once

#include <cassert>
#include "common/logger.hpp"

#define ASSERT_ERROR(i_cond, ...)                                              \
    if (!(i_cond))                                                             \
    {                                                                          \
        ln::get_logger()->error(__VA_ARGS__);                                  \
        assert(false);                                                         \
    }
