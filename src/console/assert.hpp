#pragma once

#include <cassert>

#include "common/logger.hpp"

#define LN_ASSERT(i_cond)                                                      \
    if (!(i_cond))                                                             \
    {                                                                          \
        assert(false);                                                         \
    }

#define LN_ASSERT_ERROR_COND(i_cond, ...)                                      \
    if (!(i_cond))                                                             \
    {                                                                          \
        LN_ASSERT_ERROR(__VA_ARGS__);                                          \
    }

#define LN_ASSERT_ERROR(...)                                                   \
    assert(false);                                                             \
    LN_LOG_ERROR(nh::get_logger(), __VA_ARGS__)

#define LN_ASSERT_FATAL_COND(i_cond, ...)                                      \
    if (!(i_cond))                                                             \
    {                                                                          \
        LN_ASSERT_FATAL(__VA_ARGS__);                                          \
    }

#define LN_ASSERT_FATAL(...)                                                   \
    assert(false);                                                             \
    LN_LOG_FATAL(nh::get_logger(), __VA_ARGS__)
