#pragma once

// @FIXME: spdlog will include windows header files, we need to include them
// before "glfw3.h" so that glfw won't redefine symbols.
#include "misc/logger.hpp"
#include "glfw/glfw3.h"

#include "nesish/nesish.h"
#include "nhbase/klass.hpp"

#include "imgui.h"
#include "ImGuiFileBrowser.h"

#include "app.hpp"

#include <string>
#include <unordered_map>

#include "gui/messager.hpp"
#include "input/controller.hpp"

namespace sh {

struct Renderer;
struct AudioData;
struct Resampler;
struct PCMWriter;

struct Window;

struct Application {
  public:
    Application();
    ~Application();

    NB_KLZ_DELETE_COPY_MOVE(Application);

  public:
    bool
    init(AppOpt i_opts, Logger *i_logger);
    void
    release();

    int
    run();

  private:
    void
    tick();

    void
    load_game(const std::string &i_rom_path);
    void
    release_game();

    friend struct Messager;

    bool
    running_game() const;

  private:
    int
    get_menubar_height();

    void
    draw_menubar(float *o_height = nullptr);

    static void
    key_callback(GLFWwindow *window, int vkey, int scancode, int action,
                 int mods);

  private:
    static Application *s_instance;

  private:
    AppOpt m_options;

    GLFWwindow *m_win;
    bool m_glfw_inited;

    ImGuiContext *m_imgui_ctx;
    bool m_imgui_glfw_inited;
    bool m_imgui_opengl_inited;

    imgui_addons::ImGuiFileBrowser m_file_dialog;

    Logger *m_logger;
    NHLogger m_nh_logger;
    NHConsole m_emu;
    std::string m_running_rom;

    NHController m_p1;
    NHController m_p2;

    Renderer *m_renderer;

    void *m_audio_buf;
    AudioData *m_audio_data;
    Resampler *m_resampler;
    PCMWriter *m_pcm_writer;

    bool m_paused;

    Messager m_messager;
    std::unordered_map<std::string, Window *> m_sub_wins;

    KeyMapping m_p1_keys;
    KeyMapping m_p2_keys;
};

} // namespace sh
