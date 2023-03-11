#pragma once

#include <cassert>

#include "log.hpp"

#define NH_ASSERT(i_cond)                                                      \
    if (!(i_cond))                                                             \
    {                                                                          \
        assert(false);                                                         \
    }

#define NH_ASSERT_ERROR_COND(i_cond, i_logger, ...)                            \
    if (!(i_cond))                                                             \
    {                                                                          \
        NH_ASSERT_ERROR(i_logger, __VA_ARGS__);                                \
    }

#define NH_ASSERT_ERROR(i_logger, ...)                                         \
    NH_ASSERT(false);                                                          \
    NH_LOG_ERROR(i_logger, __VA_ARGS__)

#define NH_ASSERT_FATAL_COND(i_cond, i_logger, ...)                            \
    if (!(i_cond))                                                             \
    {                                                                          \
        NH_ASSERT_FATAL(i_logger, __VA_ARGS__);                                \
    }

#define NH_ASSERT_FATAL(i_logger, ...)                                         \
    NH_ASSERT(false);                                                          \
    NH_LOG_FATAL(i_logger, __VA_ARGS__)
