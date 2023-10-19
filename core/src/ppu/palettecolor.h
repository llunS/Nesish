#pragma once

#include "types.h"

/// @brief Index color. 8-bit in memory, 6-bit in use.
typedef struct palettecolor_s {
    u8 Value;
} palettecolor_s;

/// @brief The number of representable colors
#define PALETTECOLOR_SIZE 64
