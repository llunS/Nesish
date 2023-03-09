// @FIXME: spdlog will include windows header files, we need to include them
// before "glfw3.h" so that glfw won't redefine symbols.
#include "console/emulator.hpp"
#include "debugger_window.hpp"

#include <cassert>
#include <cstdio>

#include "glfw_app/glad/glad.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glfw_app/gui/rect_cut.hpp"

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
    , m_ptn_tbl_palette(nh::Emulator::BG0)
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
DebuggerWindow::pre_render(nh::Emulator &io_emu)
{
    io_emu.set_ptn_tbl_palette_dbg(m_ptn_tbl_palette);
}

void
DebuggerWindow::post_render(nh::Emulator &io_emu)
{
    io_emu.set_debug_off(nhd::DBG_PALETTE);
    io_emu.set_debug_off(nhd::DBG_OAM);
    io_emu.set_debug_off(nhd::DBG_PATTERN);
}

void
DebuggerWindow::render(nh::Emulator &io_emu)
{
    assert(m_win);

    makeCurrent();

    if (m_resizable)
    {
        updateViewportSize();
    }

    /* Reset states at the start */
    io_emu.set_debug_off(nhd::DBG_PALETTE);
    io_emu.set_debug_off(nhd::DBG_OAM);
    io_emu.set_debug_off(nhd::DBG_PATTERN);

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
    io_emu.set_ptn_tbl_palette_dbg(m_ptn_tbl_palette);

    glfwSwapBuffers(m_win);
}

void
DebuggerWindow::draw_control(const Rect &i_layout, nh::Emulator &io_emu)
{
    ImGui::SetNextWindowPos(i_layout.pos(), ImGuiCond_Once);
    ImGui::SetNextWindowSize(i_layout.size(), ImGuiCond_Once);
    if (ImGui::Begin("Control", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        if (ImGui::Button("Power"))
        {
            io_emu.power_up();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            io_emu.reset();
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
DebuggerWindow::draw_frame_debugger(const Rect &i_layout, nh::Emulator &io_emu)
{
    ImGui::SetNextWindowPos(i_layout.pos(), ImGuiCond_Once);
    ImGui::SetNextWindowSize(i_layout.size(), ImGuiCond_Once);
    if (ImGui::Begin("Frame Debugger", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // @TODO: Change it to nametable viewer
        // draw_fd_frame(io_emu);
        // ImGui::Spacing();
        io_emu.set_debug_on(nhd::DBG_PATTERN);
        draw_fd_pattern(io_emu);
        ImGui::Spacing();
        io_emu.set_debug_on(nhd::DBG_OAM);
        draw_fd_oam(io_emu);
        ImGui::Spacing();
        io_emu.set_debug_on(nhd::DBG_PALETTE);
        draw_fd_palette(io_emu);
        ImGui::Spacing();
    }
    ImGui::End();
}

void
DebuggerWindow::draw_fd_frame(const nh::Emulator &i_emu)
{
    ImGui::PushID("<Frame>");

    ImGui::Text("<Frame>");
    const auto &framebuf = i_emu.get_frame();
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
DebuggerWindow::draw_fd_pattern(const nh::Emulator &i_emu)
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
        for (int i = 0; i < PALETTE_COUNT; i++)
        {
            if (ImGui::Selectable(names[i]))
            {
                m_ptn_tbl_palette = nh::Emulator::PaletteSet(i);
            }
        }
        ImGui::EndPopup();
    }

    auto draw_ptn_tbl = [](const nhd::PatternTable &i_tbl, sh::Texture &o_tex,
                           const char *i_title) -> void {
        ImGui::BeginGroup();

        ImGui::TextUnformatted(i_title);

        if (o_tex.from_ptn_tbl(i_tbl))
        {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            constexpr float scale = 2.f;
            ImGui::Image(
                (ImTextureID)(std::intptr_t)o_tex.texture(),
                {i_tbl.get_width() * scale, i_tbl.get_height() * scale}, {0, 0},
                {1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();

                ImGuiIO &io = ImGui::GetIO();
                float x_in_tbl = io.MousePos.x - pos.x;
                float y_in_tbl = io.MousePos.y - pos.y;
                int tile_x = static_cast<int>(x_in_tbl /
                                              (i_tbl.get_tile_width() * scale));
                int tile_y = static_cast<int>(
                    y_in_tbl / (i_tbl.get_tile_height() * scale));
                int tile_idx = tile_y * i_tbl.get_tiles_width() + tile_x;
                ImGui::Text("Tile: %d (0x%02X)", tile_idx, tile_idx);

                float x_delta = 1.0f / i_tbl.get_tiles_width();
                float y_delta = 1.0f / i_tbl.get_tiles_height();
                ImVec2 uv0 = ImVec2(tile_x * x_delta, tile_y * y_delta);
                ImVec2 uv1 = ImVec2(uv0.x + x_delta, uv0.y + y_delta);
                constexpr float zoom = 15.0f;
                ImGui::Image((ImTextureID)(std::intptr_t)o_tex.texture(),
                             ImVec2(i_tbl.get_tile_width() * zoom,
                                    i_tbl.get_tile_height() * zoom),
                             uv0, uv1, {1.0f, 1.0f, 1.0f, 1.0f},
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
    draw_ptn_tbl(i_emu.get_ptn_tbl_dbg(false), m_ptn_tbl_texs[0], "[Left]");
    ImGui::SameLine(0.0f, 20.f);
    draw_ptn_tbl(i_emu.get_ptn_tbl_dbg(true), m_ptn_tbl_texs[1], "[Right]");

    ImGui::PopID();
}

void
DebuggerWindow::draw_fd_palette(const nh::Emulator &i_emu)
{
    ImGui::PushID("<Palette>");

    ImGui::AlignTextToFramePadding();
    ImGui::Text("<Palette>");
    ImGui::SameLine();
    HelpMarker("The snapshot was took at the end of the rendering.");

    const auto &palette = i_emu.get_palette_dbg();
    static_assert(nhd::Palette::color_count() == 32, "Check fixed loop below");
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

            auto rgb_to_imvec4 = [](nhd::Color i_clr) -> ImVec4 {
                return ImVec4(i_clr.r / 255.f, i_clr.g / 255.f, i_clr.b / 255.f,
                              1.0f);
            };
            nhd::Color color = palette.get_color(palette_idx);
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
DebuggerWindow::draw_fd_oam(const nh::Emulator &i_emu)
{
    ImGui::PushID("<OAM>");

    ImGui::AlignTextToFramePadding();
    ImGui::Text("<OAM>");
    ImGui::SameLine();
    HelpMarker("The snapshot was took at the end of the rendering, and "
               "assumes OAMADDR starts with 0.");

    const auto &oam = i_emu.get_oam_dbg();
    static_assert(nhd::OAM::get_sprite_count() == 64, "Check fixed loop below");
    static_assert(sizeof(m_sp_tex) / sizeof(Texture) == 64,
                  "Check fixed loop below");
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            int k = i * 16 + j;

            const auto &sp = oam.get_sprite(k);
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
                    ImGui::Text("X: %u (0x%02X)", sp.x, sp.x);
                    ImGui::Text("Y: %u (0x%02X)", sp.y, sp.y);
                    ImGui::EndGroup();

                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::Text("Tile: %u (0x%02X)", sp.tile, sp.tile);
                    ImGui::Text("Attr: %u (0x%02X)", sp.attr, sp.attr);
                    ImGui::EndGroup();

                    ImGui::Text("SP Palette: %u", sp.palette_set());
                    ImGui::Text("Background: %u", sp.background());
                    ImGui::Text("Flip X: %u", sp.flip_x());
                    ImGui::Text("Flip Y: %u", sp.flip_y());

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
