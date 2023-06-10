#pragma once

#include "nesish/nesish.h"
#include "nhbase/klass.hpp"

#include "imgui.h"
#ifndef SH_TGT_WEB
#include "ImGuiFileBrowser.h"
#endif

#include <string>
#include <unordered_map>

#include "gui/messager.hpp"
#include "input/controller.hpp"

#include "rendering/texture.hpp"

// Current implementation on Web is far from ideal, so disable it.
#if defined(SH_TGT_WEB)
#define SH_NO_AUDIO
#endif

struct GLFWwindow;

#ifdef SH_TGT_WEB
extern "C" void
nh_on_canvas_size_changed(int, int);
extern "C" void
nh_on_game_opened(const char *);
#endif

namespace sh {

struct Renderer;
struct AudioData;
struct Resampler;
#ifndef SH_TGT_WEB
struct PCMWriter;
#endif

struct Window;

struct Application {
  public:
    Application();
    ~Application();

    NB_KLZ_DELETE_COPY_MOVE(Application);

  public:
    bool
    init();
    void
    release();

    int
    run();

  private:
    void
    tick(double i_delta_s);

    void
    load_game(const char *i_id_path, const char *i_real_path);
    void
    release_game();

    void
    on_game_opened(const char *i_id_path, const char *i_real_path);

    friend struct Messager;
#ifdef SH_TGT_WEB
    friend void ::nh_on_canvas_size_changed(int, int);
    friend void ::nh_on_game_opened(const char *);
#endif

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
    GLFWwindow *m_win;
    bool m_glfw_inited;

    ImGuiContext *m_imgui_ctx;
    bool m_imgui_glfw_inited;
    bool m_imgui_opengl_inited;

#ifndef SH_TGT_WEB
    imgui_addons::ImGuiFileBrowser m_file_dialog;
#endif

    Logger *m_logger;
    NHLogger m_nh_logger;
    NHConsole m_emu;
    std::string m_running_rom;

    NHController m_p1;
    NHController m_p2;

#ifndef SH_TGT_WEB
    Renderer *m_renderer;
#endif

#ifndef SH_NO_AUDIO
    void *m_audio_buf;
    AudioData *m_audio_data;
    Resampler *m_resampler;
#ifndef SH_TGT_WEB
    PCMWriter *m_pcm_writer;
#endif
#endif

    bool m_paused;
#ifdef SH_TGT_MACOS
    bool m_sleepless;
#endif
    bool m_muted;

    Messager m_messager;
    std::unordered_map<std::string, Window *> m_sub_wins;

    KeyMapping m_p1_keys;
    KeyMapping m_p2_keys;

    Texture m_frame_tex;
#ifdef SH_TGT_WEB
    Texture m_black_frm_tex;
#endif
};

} // namespace sh
