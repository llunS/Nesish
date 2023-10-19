#pragma once

#include <assert.h>

#include "log.h"

#define NH_ASSERT(cond) assert(cond)

#define ASSERT_ERROR(logger, ...)                                              \
    NH_ASSERT(0);                                                              \
    LOG_ERROR(logger, __VA_ARGS__)

#define ASSERT_FATAL(logger, ...)                                              \
    NH_ASSERT(0);                                                              \
    LOG_FATAL(logger, __VA_ARGS__)
