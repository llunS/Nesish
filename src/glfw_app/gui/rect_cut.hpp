#pragma once

#include "imgui.h"

namespace ln_app {

// Reference: https://halt.software/dead-simple-layouts/

struct Rect {
  public:
    Rect(float i_minx, float i_miny, float i_maxx, float i_maxy);
    Rect(const ImVec2 &i_pos, const ImVec2 &i_size);

  public:
    ImVec2
    pos() const;
    ImVec2
    size() const;

  public:
    float minx, miny, maxx, maxy;
};

Rect
cut_left(Rect &io_rect, float i_amount);

Rect
cut_right(Rect &io_rect, float i_amount);

Rect
cut_top(Rect &io_rect, float i_amount);

Rect
cut_bottom(Rect &io_rect, float i_amount);

} // namespace ln_app
