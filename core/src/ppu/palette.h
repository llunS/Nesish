#pragma once

#include "ppu/palettecolor.h"
#include "ppu/color.h"

typedef struct palette_s {
    color_s (*ToRgb)(palettecolor_s color);
} palette_s;
