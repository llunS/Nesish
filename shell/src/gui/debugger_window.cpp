#include "debugger_window.hpp"

#include <cassert>
#include <cstdio>

#include "glad.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "gui/rect_cut.hpp"

#include <string>

namespace sh {

static void
HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

} // namespace sh

namespace sh {

DebuggerWindow::DebuggerWindow()
    : m_imgui_ctx(nullptr)
    , m_paused(false)
    , m_should_quit(false)
    , m_sp_tex{}
    , m_ptn_tbl_texs{}
    , m_ptn_tbl_palette(NHD_PALETTE_BG0)
{
}

void
DebuggerWindow::release()
{
    if (m_imgui_ctx)
    {
        ImGui::SetCurrentContext(m_imgui_ctx);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext(m_imgui_ctx);
    }

    PlatformWindow::release();
}

bool
DebuggerWindow::post_init()
{
    assert(m_win);

    /* imgui setup */
    glfwMakeContextCurrent(m_win); // need to call gl functions (maybe).

    IMGUI_CHECKVERSION();
    m_imgui_ctx = ImGui::CreateContext();
    if (!m_imgui_ctx)
    {
        return false;
    }

    ImGui::SetCurrentContext(m_imgui_ctx);
    ImGui_ImplGlfw_InitForOpenGL(m_win, true);
    const char *glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);

    return true;
}

void
DebuggerWindow::makeCurrent()
{
    PlatformWindow::makeCurrent();

    if (m_imgui_ctx)
    {
        ImGui::SetCurrentContext(m_imgui_ctx);
    }
}

void
DebuggerWindow::pre_render(NHConsole io_emu)
{
    nhd_set_ptn_table_palette(io_emu, m_ptn_tbl_palette);
}

void
DebuggerWindow::post_render(NHConsole io_emu)
{
    nhd_turn_debug_off(io_emu, NHD_DBG_PALETTE);
    nhd_turn_debug_off(io_emu, NHD_DBG_OAM);
    nhd_turn_debug_off(io_emu, NHD_DBG_PATTERN);
}

void
DebuggerWindow::render(NHConsole io_emu)
{
    assert(m_win);

    makeCurrent();

    if (m_resizable)
    {
        updateViewportSize();
    }

    /* Reset states at the start */
    nhd_turn_debug_off(io_emu, NHD_DBG_PALETTE);
    nhd_turn_debug_off(io_emu, NHD_DBG_OAM);
    nhd_turn_debug_off(io_emu, NHD_DBG_PATTERN);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    Rect layout_win = {viewport->WorkPos, viewport->WorkSize};

    /* Control */
    Rect layout_ctrl = cut_top(layout_win, 55);
    draw_control(layout_ctrl, io_emu);
    /* Frame Debugger */
    Rect layout_fd = layout_win;
    draw_frame_debugger(layout_fd, io_emu);

    // End the Dear ImGui frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    /* Change emulater states based on user input */
    nhd_set_ptn_table_palette(io_emu, m_ptn_tbl_palette);

    glfwSwapBuffers(m_win);
}

void
DebuggerWindow::draw_control(const Rect &i_layout, NHConsole io_emu)
{
    ImGui::SetNextWindowPos(i_layout.pos(), ImGuiCond_Once);
    ImGui::SetNextWindowSize(i_layout.size(), ImGuiCond_Once);
    if (ImGui::Begin("Control", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        if (ImGui::Button("Power"))
        {
            nh_power_up(io_emu);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            nh_reset(io_emu);
        }

        ImGui::SameLine(0.0f, 15.f);
        if (m_paused)
        {
            if (ImGui::Button("Play"))
            {
                m_paused = !m_paused;
            }
        }
        else
        {
            if (ImGui::Button("Pause"))
            {
                m_paused = !m_paused;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Quit"))
        {
            m_should_quit = true;
        }
    }
    ImGui::End();
}

void
DebuggerWindow::draw_frame_debugger(const Rect &i_layout, NHConsole io_emu)
{
    ImGui::SetNextWindowPos(i_layout.pos(), ImGuiCond_Once);
    ImGui::SetNextWindowSize(i_layout.size(), ImGuiCond_Once);
    if (ImGui::Begin("Frame Debugger", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // @TODO: Change it to nametable viewer
        // draw_fd_frame(io_emu);
        // ImGui::Spacing();
        nhd_turn_debug_on(io_emu, NHD_DBG_PATTERN);
        draw_fd_pattern(io_emu);
        ImGui::Spacing();
        nhd_turn_debug_on(io_emu, NHD_DBG_OAM);
        draw_fd_oam(io_emu);
        ImGui::Spacing();
        nhd_turn_debug_on(io_emu, NHD_DBG_PALETTE);
        draw_fd_palette(io_emu);
        ImGui::Spacing();
    }
    ImGui::End();
}

void
DebuggerWindow::draw_fd_frame(NHConsole i_emu)
{
    ImGui::PushID("<Frame>");

    ImGui::Text("<Frame>");
    NHFrame framebuf = nh_get_frm(i_emu);
    if (m_frame_tex.from_frame(framebuf))
    {
        ImGui::Image(
            (ImTextureID)(std::intptr_t)m_frame_tex.texture(),
            {float(m_frame_tex.get_width()), float(m_frame_tex.get_height())},
            {0, 0}, {1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});
    }
    else
    {
        ImGui::Text("[X]");
    }

    ImGui::PopID();
}

void
DebuggerWindow::draw_fd_pattern(NHConsole i_emu)
{
    ImGui::PushID("<Pattern>");

    ImGui::AlignTextToFramePadding();
    ImGui::Text("<Pattern>");

    ImGui::SameLine();
    constexpr int PALETTE_COUNT = 8;
    constexpr const char *names[PALETTE_COUNT] = {"BG0", "BG1", "BG2", "BG3",
                                                  "SP0", "SP1", "SP2", "SP3"};
    static_assert(names[PALETTE_COUNT - 1] && PALETTE_COUNT >= 1,
                  "Incorrect count of Palette set");
    char buf[64];
    (void)snprintf(buf, sizeof(buf), "%s###ptn_tbl_palette_trigger",
                   names[m_ptn_tbl_palette]);
    if (ImGui::Button(buf))
    {
        ImGui::OpenPopup("ptn_tbl_palette_popup");
    }
    if (ImGui::BeginPopup("ptn_tbl_palette_popup"))
    {
        for (NHDPaletteSet i = 0; i < PALETTE_COUNT; i++)
        {
            if (ImGui::Selectable(names[i]))
            {
                m_ptn_tbl_palette = i;
            }
        }
        ImGui::EndPopup();
    }

    auto draw_ptn_tbl = [](NHDPatternTable i_tbl, sh::Texture &o_tex,
                           const char *i_title) -> void {
        ImGui::BeginGroup();

        ImGui::TextUnformatted(i_title);

        if (o_tex.from_ptn_tbl(i_tbl))
        {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            constexpr float scale = 2.f;
            ImGui::Image((ImTextureID)(std::intptr_t)o_tex.texture(),
                         {nhd_ptn_table_width(i_tbl) * scale,
                          nhd_ptn_table_height(i_tbl) * scale},
                         {0, 0}, {1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();

                auto tile_w = nhd_ptn_table_tile_width(i_tbl);
                auto tile_h = nhd_ptn_table_tile_height(i_tbl);
                auto tiles_w = nhd_ptn_table_tiles_width(i_tbl);
                auto tiles_h = nhd_ptn_table_tiles_height(i_tbl);

                ImGuiIO &io = ImGui::GetIO();
                float x_in_tbl = io.MousePos.x - pos.x;
                float y_in_tbl = io.MousePos.y - pos.y;
                int tile_x = static_cast<int>(x_in_tbl / (tile_w * scale));
                int tile_y = static_cast<int>(y_in_tbl / (tile_h * scale));
                int tile_idx = tile_y * tiles_w + tile_x;
                ImGui::Text("Tile: %d (0x%02X)", tile_idx, tile_idx);

                float x_delta = 1.0f / tiles_w;
                float y_delta = 1.0f / tiles_h;
                ImVec2 uv0 = ImVec2(tile_x * x_delta, tile_y * y_delta);
                ImVec2 uv1 = ImVec2(uv0.x + x_delta, uv0.y + y_delta);
                constexpr float zoom = 15.0f;
                ImGui::Image((ImTextureID)(std::intptr_t)o_tex.texture(),
                             ImVec2(tile_w * zoom, tile_h * zoom), uv0, uv1,
                             {1.0f, 1.0f, 1.0f, 1.0f},
                             {1.0f, 1.0f, 1.0f, 1.0f});

                ImGui::EndTooltip();
            }
        }
        else
        {
            ImGui::Text("[X]");
        }

        ImGui::EndGroup();
    };

    static_assert(sizeof(m_ptn_tbl_texs) / sizeof(Texture) == 2,
                  "Invalid array index");
    draw_ptn_tbl(nhd_get_ptn_table(i_emu, false), m_ptn_tbl_texs[0], "[Left]");
    ImGui::SameLine(0.0f, 20.f);
    draw_ptn_tbl(nhd_get_ptn_table(i_emu, true), m_ptn_tbl_texs[1], "[Right]");

    ImGui::PopID();
}

void
DebuggerWindow::draw_fd_palette(NHConsole i_emu)
{
    ImGui::PushID("<Palette>");

    ImGui::AlignTextToFramePadding();
    ImGui::Text("<Palette>");
    ImGui::SameLine();
    HelpMarker("The snapshot was took at the end of the rendering.");

    NHDPalette palette = nhd_get_palette(i_emu);
    static_assert(NHD_PALETTE_COLORS == 32, "Check fixed loop below");
    char const *const pa_rows[2] = {"Background", "Sprite"};
    float lock_x = 0.0;
    for (int r = 0; r < 2; ++r)
    {
        const char *row = pa_rows[r];

        ImGui::PushID(row);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(row);
        ImGui::SameLine(0.0f, 20.f);
        if (r <= 0)
        {
            lock_x = ImGui::GetCursorPosX();
        }
        else
        {
            ImGui::SetCursorPosX(lock_x);
        }
        for (int c = 0; c < 16; ++c)
        {
            int palette_idx = r * 16 + c;

            ImGui::PushID(c);

            auto rgb_to_imvec4 = [](NHDColor i_clr) -> ImVec4 {
                return ImVec4(i_clr.r / 255.f, i_clr.g / 255.f, i_clr.b / 255.f,
                              1.0f);
            };
            NHDColor color = nhd_palette_color(palette, palette_idx);
            ImVec4 im_color = rgb_to_imvec4(color);

            if (c % 4 == 0)
            {
                std::string s = std::to_string(c / 4);
                ImGui::TextUnformatted(s.c_str());
                ImGui::SameLine();
            }

            ImGui::ColorButton("##color", im_color,
                               ImGuiColorEditFlags_NoBorder |
                                   ImGuiColorEditFlags_NoAlpha |
                                   ImGuiColorEditFlags_NoTooltip);
            if ((c + 1) % 4 != 0)
            {
                ImGui::SameLine(0.0f, 0.0f);
            }
            else
            {
                ImGui::SameLine(0.0f, 25.f);
            }

            // details tooltip
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::ColorButton("##color", im_color,
                                   ImGuiColorEditFlags_NoBorder |
                                       ImGuiColorEditFlags_NoAlpha |
                                       ImGuiColorEditFlags_NoTooltip,
                                   ImVec2(40, 40));
                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::Text("%u (0x%02X)", color.index, color.index);
                ImGui::Text("(%d, %d, %d)", color.r, color.g, color.b);
                ImGui::EndGroup();
                ImGui::EndTooltip();
            }

            ImGui::PopID();
        }
        ImGui::NewLine();
        ImGui::PopID();
    }

    ImGui::PopID();
}

void
DebuggerWindow::draw_fd_oam(NHConsole i_emu)
{
    ImGui::PushID("<OAM>");

    ImGui::AlignTextToFramePadding();
    ImGui::Text("<OAM>");
    ImGui::SameLine();
    HelpMarker("The snapshot was took at the end of the rendering, and "
               "assumes OAMADDR starts with 0.");

    NHDOAM oam = nhd_get_oam(i_emu);
    static_assert(NHD_OAM_SPRITES == 64, "Check fixed loop below");
    static_assert(sizeof(m_sp_tex) / sizeof(Texture) == 64,
                  "Check fixed loop below");
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            int k = i * 16 + j;

            NHDSprite sp = nhd_oam_sprite(oam, k);
            if (m_sp_tex[k].from_sprite(sp))
            {
                constexpr float scale = 3.f;
                ImGui::Image((ImTextureID)(std::intptr_t)m_sp_tex[k].texture(),
                             {float(m_sp_tex[k].get_width() * scale),
                              float(m_sp_tex[k].get_height() * scale)},
                             {0, 0}, {1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});

                // details tooltip
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();

                    constexpr float scale_details = 12.f;
                    ImGui::Image(
                        (ImTextureID)(std::intptr_t)m_sp_tex[k].texture(),
                        {float(m_sp_tex[k].get_width() * scale_details),
                         float(m_sp_tex[k].get_height() * scale_details)},
                        {0, 0}, {1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});

                    ImGui::SameLine();
                    ImGui::BeginGroup();

                    ImGui::BeginGroup();
                    auto sp_x = nhd_sprite_x(sp);
                    auto sp_y = nhd_sprite_y(sp);
                    ImGui::Text("X: %u (0x%02X)", sp_x, sp_x);
                    ImGui::Text("Y: %u (0x%02X)", sp_y, sp_y);
                    ImGui::EndGroup();

                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    auto sp_tile = nhd_sprite_tile(sp);
                    auto sp_attr = nhd_sprite_attr(sp);
                    ImGui::Text("Tile: %u (0x%02X)", sp_tile, sp_tile);
                    ImGui::Text("Attr: %u (0x%02X)", sp_attr, sp_attr);
                    ImGui::EndGroup();

                    ImGui::Text("SP Palette: %u", nhd_sprite_palette_set(sp));
                    ImGui::Text("Background: %u", nhd_sprite_background(sp));
                    ImGui::Text("Flip X: %u", nhd_sprite_flip_x(sp));
                    ImGui::Text("Flip Y: %u", nhd_sprite_flip_y(sp));

                    ImGui::EndGroup();

                    ImGui::EndTooltip();
                }
            }
            else
            {
                ImGui::Text("[X]");
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
    }

    ImGui::PopID();
}

bool
DebuggerWindow::isPaused() const
{
    return m_paused;
}

bool
DebuggerWindow::shouldQuit() const
{
    return m_should_quit;
}

} // namespace sh
