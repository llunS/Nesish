#include "rect_cut.hpp"

#include <algorithm>

namespace ln_app {

Rect::Rect(float i_minx, float i_miny, float i_maxx, float i_maxy)
    : minx(i_minx)
    , miny(i_miny)
    , maxx(i_maxx)
    , maxy(i_maxy)
{
}

Rect::Rect(const ImVec2 &i_pos, const ImVec2 &i_size)
    : Rect(i_pos.x, i_pos.y, i_pos.x + i_size.x, i_pos.y + i_size.y)
{
}

ImVec2
Rect::pos() const
{
    return ImVec2(minx, miny);
}
ImVec2
Rect::size() const
{
    return ImVec2(maxx - minx, maxy - miny);
}

Rect
cut_left(Rect &io_rect, float i_amount)
{
    float minx = io_rect.minx;
    io_rect.minx = std::min(io_rect.maxx, io_rect.minx + i_amount);
    return {minx, io_rect.miny, io_rect.minx, io_rect.maxy};
}

Rect
cut_right(Rect &io_rect, float i_amount)
{
    float maxx = io_rect.maxx;
    io_rect.maxx = std::max(io_rect.minx, io_rect.maxx - i_amount);
    return {io_rect.maxx, io_rect.miny, maxx, io_rect.maxy};
}

Rect
cut_top(Rect &io_rect, float i_amount)
{
    float miny = io_rect.miny;
    io_rect.miny = std::min(io_rect.maxy, io_rect.miny + i_amount);
    return {io_rect.minx, miny, io_rect.maxx, io_rect.miny};
}

Rect
cut_bottom(Rect &io_rect, float i_amount)
{
    float maxy = io_rect.maxy;
    io_rect.maxy = std::max(io_rect.miny, io_rect.maxy - i_amount);
    return {io_rect.minx, io_rect.maxy, io_rect.maxx, maxy};
}

} // namespace ln_app
