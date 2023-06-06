#include "ppu_debugger.hpp"

#include "gui/imgui_utils.hpp"
#include "gui/messager.hpp"

namespace sh {

PPUDebugger::PPUDebugger(const std::string &i_name, NHConsole io_emu,
                         Messager *i_messager)
    : Window(i_name, io_emu, i_messager)
    , m_sp_tex{}
    , m_ptn_tbl_texs{}
    , m_ptn_tbl_palette(NHD_PALETTE_BG0)
{
    nhd_set_ptn_table_palette(m_emu, m_ptn_tbl_palette);
}

PPUDebugger::~PPUDebugger() {}

void
PPUDebugger::render()
{
    nhd_turn_debug_off(m_emu, NHD_DBG_PALETTE);
    nhd_turn_debug_off(m_emu, NHD_DBG_OAM);
    nhd_turn_debug_off(m_emu, NHD_DBG_PATTERN);

    if (m_open)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
        if (ImGui::Begin(m_name.c_str(), &m_open,
                         ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (m_messager->running_game())
            {
                // @TODO: Nametable viewer

                draw_pattern();
                nhd_turn_debug_on(m_emu, NHD_DBG_PATTERN);

                ImGui::Spacing();
                draw_oam();
                nhd_turn_debug_on(m_emu, NHD_DBG_OAM);

                ImGui::Spacing();
                draw_palette();
                nhd_turn_debug_on(m_emu, NHD_DBG_PALETTE);
            }
            else
            {
                ImGui::Text("No game is running");
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}

void
PPUDebugger::draw_pattern()
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
                nhd_set_ptn_table_palette(m_emu, m_ptn_tbl_palette);
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
    draw_ptn_tbl(nhd_get_ptn_table(m_emu, false), m_ptn_tbl_texs[0], "[Left]");
    ImGui::SameLine(0.0f, 20.f);
    draw_ptn_tbl(nhd_get_ptn_table(m_emu, true), m_ptn_tbl_texs[1], "[Right]");

    ImGui::PopID();
}

void
PPUDebugger::draw_palette()
{
    ImGui::PushID("<Palette>");

    ImGui::AlignTextToFramePadding();
    ImGui::Text("<Palette>");
    ImGui::SameLine();
    HelpMarker("The snapshot was took at the end of the rendering.");

    NHDPalette palette = nhd_get_palette(m_emu);
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
                if (c + 1 < 16)
                {
                    ImGui::SameLine(0.0f, 25.f);
                }
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
        ImGui::PopID();
    }

    ImGui::PopID();
}

void
PPUDebugger::draw_oam()
{
    ImGui::PushID("<OAM>");

    ImGui::AlignTextToFramePadding();
    ImGui::Text("<OAM>");
    ImGui::SameLine();
    HelpMarker("The snapshot was took at the end of the rendering, and "
               "assumes OAMADDR starts with 0.");

    NHDOAM oam = nhd_get_oam(m_emu);
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
            if (j + 1 < 16)
            {
                ImGui::SameLine();
            }
        }
    }

    ImGui::PopID();
}

} // namespace sh
