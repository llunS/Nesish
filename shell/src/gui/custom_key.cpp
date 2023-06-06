#include "custom_key.hpp"

#include "gui/messager.hpp"
#include "misc/config.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "gui/imgui_utils.hpp"

#include <unordered_map>

namespace sh {

static constexpr ImVec2 KEY_BUTTON_SIZE =
    ImVec2(4.5f, 4.5f); // Enough for at most 6 letters

static constexpr int
keyname_max_len()
{
    return 10; // More than enough
}
static const char *
pv_vkey_to_keyname(VirtualKey i_vkey);
static bool
pv_is_key_valid(VirtualKey i_vkey);

CustomKey::CustomKey(const std::string &i_name, NHConsole io_emu,
                     Messager *i_messager)
    : Window(i_name, io_emu, i_messager)
    , m_recording(false)
{
}

CustomKey::~CustomKey() {}

void
CustomKey::render()
{
    if (m_open)
    {
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Once);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 14.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                            ImVec2(ImGui::GetStyle().ItemSpacing.x, 10.0f));
        if (ImGui::Begin(m_name.c_str(), &m_open, ImGuiWindowFlags_NoResize))
        {
            draw_key_config(NH_CTRL_P1, "Player 1");

            ImGui::Spacing();
            draw_key_config(NH_CTRL_P2, "Player 2");

            ImGui::Spacing();
            {
                auto content_w = ImGui::GetContentRegionAvail().x;
                auto text_w = ImGui::CalcTextSize("Reset Both To Default").x;
                ImGui::SetCursorPosX((content_w - text_w) * 0.5f);
                if (ImGui::Button("Reset Both To Default"))
                {
                    KeyMapping p1, p2;
                    if (reset_default_key_config(&p1, &p2,
                                                 m_messager->get_logger()))
                    {
                        m_messager->reset_key_mapping(p1, p2);
                    }
                }
            }
        }
        else
        {
            m_recording = false;
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
    }
    else
    {
        m_recording = false;
    }
}

void
CustomKey::draw_key_config(NHCtrlPort i_port, const char *i_port_name)
{
    ImGui::PushID(i_port_name);

    ImGui::TextUnformatted(i_port_name);
    ImGui::SameLine();
    HelpMarker("Click to set key, ESC to cancel recording.");

    char key_label[keyname_max_len() + 10];
    const KeyMapping &mapping = m_messager->get_key_mapping(i_port);
    float font_size = ImGui::GetFontSize();
    ImVec2 key_cell_size = KEY_BUTTON_SIZE * font_size;
    ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
    float cell_w_with_spacing = key_cell_size.x + spacing.x;
    float cell_h_with_spacing = key_cell_size.y + spacing.y;
    float group_spacing_x = font_size;
    ImVec2 cursor_start = ImGui::GetCursorPos();

    struct KeyButtonData {
        int x, y, z;
        NHKey nhkey;
        const char *id;
    };
    constexpr KeyButtonData KEY_BUTTONS[NH_KEYS] = {
        {1, 0, 0, NH_KEY_UP, "Up"},         {0, 0, 1, NH_KEY_LEFT, "Left"},
        {2, 0, 1, NH_KEY_RIGHT, "Right"},   {1, 0, 2, NH_KEY_DOWN, "Down"},
        {3, 1, 2, NH_KEY_SELECT, "Select"}, {4, 1, 2, NH_KEY_START, "Start"},
        {5, 2, 2, NH_KEY_B, "B"},           {6, 2, 2, NH_KEY_A, "A"},
    };
    ImGui::Spacing();
    for (NHKey i = NH_KEY_BEGIN; i != NH_KEY_END; ++i)
    {
        ImGui::SetCursorPos(cursor_start +
                            ImVec2(cell_w_with_spacing * KEY_BUTTONS[i].x +
                                       group_spacing_x * KEY_BUTTONS[i].y,
                                   cell_h_with_spacing * KEY_BUTTONS[i].z));
        (void)snprintf(key_label, sizeof(key_label), "%s###%s",
                       !(m_recording && m_recording_port == i_port &&
                         KEY_BUTTONS[i].nhkey == m_recording_key)
                           ? pv_vkey_to_keyname(mapping[KEY_BUTTONS[i].nhkey])
                           : "...",
                       KEY_BUTTONS[i].id);
        if (ImGui::Button(key_label, key_cell_size))
        {
            on_key_clicked(i_port, KEY_BUTTONS[i].nhkey);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(KEY_BUTTONS[i].id);
            ImGui::EndTooltip();
        }
    }

    ImGui::PopID();
}

void
CustomKey::on_key_clicked(NHCtrlPort i_port, NHKey i_key)
{
    // Into recording state
    m_recording = true;
    m_recording_port = i_port;
    m_recording_key = i_key;
}

bool
CustomKey::on_key_released(VirtualKey i_vkey, NHCtrlPort &o_port, NHKey &o_key)
{
    if (m_recording)
    {
        if (i_vkey == GLFW_KEY_ESCAPE)
        {
            m_recording = false;
            return false;
        }

        if (pv_is_key_valid(i_vkey))
        {
            if (save_key_config(m_recording_port, m_recording_key, i_vkey,
                                m_messager->get_logger()))
            {
                m_recording = false;

                o_port = m_recording_port;
                o_key = m_recording_key;
                return true;
            }
        }
    }
    return false;
}

/* clang-format off */
static const std::unordered_map<VirtualKey, const char *> g_vkey_to_keyname{
/* Printable keys */
{GLFW_KEY_SPACE, "SPACE"},
{GLFW_KEY_APOSTROPHE, "'"},
{GLFW_KEY_COMMA, ", "},
{GLFW_KEY_MINUS, "-"},
{GLFW_KEY_PERIOD, "."},
{GLFW_KEY_SLASH, "/"},
{GLFW_KEY_0, "0"},
{GLFW_KEY_1, "1"},
{GLFW_KEY_2, "2"},
{GLFW_KEY_3, "3"},
{GLFW_KEY_4, "4"},
{GLFW_KEY_5, "5"},
{GLFW_KEY_6, "6"},
{GLFW_KEY_7, "7"},
{GLFW_KEY_8, "8"},
{GLFW_KEY_9, "9"},
{GLFW_KEY_SEMICOLON, ";"},
{GLFW_KEY_EQUAL, "="},
{GLFW_KEY_A, "A"},
{GLFW_KEY_B, "B"},
{GLFW_KEY_C, "C"},
{GLFW_KEY_D, "D"},
{GLFW_KEY_E, "E"},
{GLFW_KEY_F, "F"},
{GLFW_KEY_G, "G"},
{GLFW_KEY_H, "H"},
{GLFW_KEY_I, "I"},
{GLFW_KEY_J, "J"},
{GLFW_KEY_K, "K"},
{GLFW_KEY_L, "L"},
{GLFW_KEY_M, "M"},
{GLFW_KEY_N, "N"},
{GLFW_KEY_O, "O"},
{GLFW_KEY_P, "P"},
{GLFW_KEY_Q, "Q"},
{GLFW_KEY_R, "R"},
{GLFW_KEY_S, "S"},
{GLFW_KEY_T, "T"},
{GLFW_KEY_U, "U"},
{GLFW_KEY_V, "V"},
{GLFW_KEY_W, "W"},
{GLFW_KEY_X, "X"},
{GLFW_KEY_Y, "Y"},
{GLFW_KEY_Z, "Z"},
{GLFW_KEY_LEFT_BRACKET, "["},
{GLFW_KEY_BACKSLASH, "\\"},
{GLFW_KEY_RIGHT_BRACKET, "]"},
{GLFW_KEY_GRAVE_ACCENT, "`"},

/* Function keys */
{GLFW_KEY_ENTER, "ENTER"},
{GLFW_KEY_TAB, "TAB"},
{GLFW_KEY_BACKSPACE, "BS"},
{GLFW_KEY_RIGHT, "RIGHT"},
{GLFW_KEY_LEFT, "LEFT"},
{GLFW_KEY_DOWN, "DOWN"},
{GLFW_KEY_UP, "UP"},
{GLFW_KEY_CAPS_LOCK, "CAPS"},
{GLFW_KEY_KP_0, "N 0"},
{GLFW_KEY_KP_1, "N 1"},
{GLFW_KEY_KP_2, "N 2"},
{GLFW_KEY_KP_3, "N 3"},
{GLFW_KEY_KP_4, "N 4"},
{GLFW_KEY_KP_5, "N 5"},
{GLFW_KEY_KP_6, "N 6"},
{GLFW_KEY_KP_7, "N 7"},
{GLFW_KEY_KP_8, "N 8"},
{GLFW_KEY_KP_9, "N 9"},
{GLFW_KEY_KP_DECIMAL, "N ."},
{GLFW_KEY_KP_DIVIDE, "N /"},
{GLFW_KEY_KP_MULTIPLY, "N *"},
{GLFW_KEY_KP_SUBTRACT, "N -"},
{GLFW_KEY_KP_ADD, "N +"},
{GLFW_KEY_KP_ENTER, "N ENTER" },
{GLFW_KEY_KP_EQUAL, "N ="},
{GLFW_KEY_LEFT_SHIFT, "LSHIFT"},
{GLFW_KEY_LEFT_CONTROL, "LCTRL"},
{GLFW_KEY_LEFT_ALT, "LALT"},
{GLFW_KEY_LEFT_SUPER, "LCMD"},
{GLFW_KEY_RIGHT_SHIFT, "RSHIFT"},
{GLFW_KEY_RIGHT_CONTROL, "RCTRL"},
{GLFW_KEY_RIGHT_ALT, "RALT"},
{GLFW_KEY_RIGHT_SUPER, "RCMD"},
{GLFW_KEY_MENU, "MENU"},
};
/* clang-format on */

const char *
pv_vkey_to_keyname(VirtualKey i_vkey)
{
    auto it = g_vkey_to_keyname.find(i_vkey);
    if (it != g_vkey_to_keyname.cend())
    {
        return it->second;
    }
    return "";
}

bool
pv_is_key_valid(VirtualKey i_vkey)
{
    return g_vkey_to_keyname.find(i_vkey) != g_vkey_to_keyname.cend();
}

} // namespace sh
