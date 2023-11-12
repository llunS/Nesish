#include "application.hpp"

#if defined(SH_TGT_WEB)
#define SH_MUTED_DEF 1
#else
#define SH_MUTED_DEF 0
#endif

#define SH_SAVE_IMGUI_INI 1
#define SH_IMGUI_INI_FILEPATH "imgui.ini"

#if defined(SH_TGT_WEB) && SH_SAVE_IMGUI_INI
#include "misc/web_utils.hpp"
#endif

#include <cstdio>
#include <chrono>
#ifdef SH_TGT_MACOS
#include <thread>
#endif
#include <cmath>

#include "audio/resampler.hpp"
#ifndef SH_TGT_WEB
#include "audio/pcm_writer.hpp"
#endif
#include "audio/channel.hpp"
#include "audio/backend.hpp"

#include "nhbase/path.hpp"

// @FIXME: spdlog will include windows header files, we need to include them
// before "glfw3.h" so that glfw won't redefine symbols.
#include "misc/logger.hpp"
#include "GLFW/glfw3.h"
#include "misc/glfunc.hpp"
#ifndef SH_TGT_WEB
#include "rendering/renderer.hpp"
#endif

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "misc/config.hpp"
#include "misc/exception.hpp"

#include "gui/ppu_debugger.hpp"
#include "gui/custom_key.hpp"

#ifdef SH_TGT_WEB

#include <emscripten.h>

static bool g_em_loop_inited = false;
static double g_em_loop_time;
static void
on_em_tick(sh::Application *app, double now);

#ifndef SH_EXPLICIT_RAF

#include <functional>

static std::function<void()> g_em_loop_cb;
static void
em_loop_cb_c()
{
    g_em_loop_cb();
};

#else

#include <emscripten/html5.h>

static EM_BOOL
em_raf_cb(double now, void *user_data)
{
    sh::Application *app = static_cast<sh::Application *>(user_data);
    on_em_tick(app, now);
    // The loop never ends.
    return EM_TRUE;
}

#endif

EM_JS(int, js_canvas_width, (), { return Module.canvas.width; });
EM_JS(int, js_canvas_height, (), { return Module.canvas.height; });
EM_JS(void, js_open_game_popup, (), { Module.openGameInput.click(); };)

extern "C" void EMSCRIPTEN_KEEPALIVE
nh_on_canvas_size_changed(int width, int height);

extern "C" void EMSCRIPTEN_KEEPALIVE
nh_on_game_opened(const char *basename);

#endif

sh::Application *g_app = nullptr;

namespace sh
{

#define PPU_DEBUGGER_NAME "PPU"
#define CUSTOM_KEY_NAME "Key Mapping"

#define TARGET_WIN_WIDTH (NH_NES_WIDTH * 2)
#define TARGET_WIN_HEIGHT (NH_NES_HEIGHT * 2)

#define DEBUG_AUDIO 0
#define DEBUG_AUDIO_PCM 0 // Record audio pcm file

#define FRAME_TIME (1.0 / 60.0)
#define AUDIO_SAMPLE_RATE 48000 // Most common rate for audio hardware
#define AUDIO_BUF_SIZE 512      // Close to 1 frame worth of buffer
// 800 = 1 / 60 * 48000, x2 for peak storage
#define AUDIO_CH_SIZE (800 * 2)

#define CONFIG_SECTION_DEBUG "Debug"
#define CONFIG_KEY_SLEEPLESS "Sleepless"
#define CONFIG_KEY_MUTED "Muted"

typedef float sample_t;
typedef Channel<sample_t, AUDIO_CH_SIZE> AudioBuffer;

#define to_AudioBuffer(ptr) (static_cast<AudioBuffer *>(ptr))

struct AudioData {
    sample_t prev;
    AudioBuffer *buf;
    Logger *logger;
    bool stopped;
};

static void
error_callback(int error, const char *description);

static void
pv_log(NHLogLevel level, const char *msg, void *user);

#ifndef SH_USE_SOKOL_AUDIO
static int
audio_playback(void *output_buffer, void *, unsigned int num_frames, double,
               RtAudioStreamStatus status, void *user_data);
#else
static void
audio_playback(float *output_buffer, int num_frames, int num_channels,
               void *user_data);
#endif

static void
pv_strobe(int enabled, void *user);
static int
pv_report(void *user);
static void
pv_reset(void *user);

#define ASM_CTRL(ctrl)                                                         \
    {                                                                          \
        ctrl.strobe = pv_strobe;                                               \
        ctrl.report = pv_report;                                               \
        ctrl.reset = pv_reset;                                                 \
    }

Application::Application()
    : m_win(nullptr)
    , m_glfw_inited(false)
    , m_imgui_ctx(nullptr)
    , m_imgui_glfw_inited(false)
    , m_imgui_opengl_inited(false)
    , m_logger(nullptr)
    , m_emu(NH_NULL)
#ifndef SH_TGT_WEB
    , m_renderer(nullptr)
#endif
#if !SH_NO_AUDIO
    , m_audio_buf(nullptr)
    , m_audio_data(nullptr)
    , m_resampler(nullptr)
#ifndef SH_TGT_WEB
    , m_pcm_writer(nullptr)
#endif
#endif
    , m_paused(false)
#ifdef SH_TGT_MACOS
    , m_sleepless(false)
#endif
    , m_muted(false)
    , m_messager(this)
{
    m_p1.user = nullptr;
    m_p2.user = nullptr;
}

Application::~Application()
{
}

bool
Application::init()
{
    SH_TRY
    {
        NHLogLevel log_level = SH_DEFAULT_LOG_LEVEL;
        (void)sh::load_log_level(log_level);
        m_logger = new sh::Logger(log_level);
    }
    SH_CATCH(const std::exception &)
    {
        goto l_err;
    }

    /* load key config */
    if (!load_key_config(m_p1_keys, m_p2_keys, m_logger)) {
        goto l_err;
    }

    /* create emulator instance */
    m_nh_logger.log = pv_log;
    m_nh_logger.user = m_logger;
    m_nh_logger.active = m_logger->level;
    m_emu = nh_new_console(&m_nh_logger);
    if (!NH_VALID(m_emu)) {
        goto l_err;
    }

    /* init glfw */
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        goto l_err;
    }
    m_glfw_inited = true;

    /* create glfw window */
#if defined(SH_TGT_WEB)
    // GL ES 2.0 / WebGL 1.0
    static const char *glsl_version = "#version 100";
    // https://emscripten.org/docs/optimizing/Optimizing-WebGL.html#which-gl-mode-to-target
    // https://emscripten.org/docs/optimizing/Optimizing-WebGL.html#migrating-to-webgl-2
    // The link suggests that 2.0 has improvements over 1.0, try it without
    // profiling
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
    // OpenGL 3.3
    static const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(SH_TGT_MACOS)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true); // Required on Mac
#endif
#endif
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_RESIZABLE, false);
    // hide first, cos we are about to adjust its size
    glfwWindowHint(GLFW_VISIBLE, false);
#ifdef SH_TGT_WEB
    // GLFW in Emscripten supports only 1 window
    // So GUI style/layout differs from it is on desktop
    m_win = glfwCreateWindow(js_canvas_width(), js_canvas_height(), "Nesish",
                             NULL, NULL);
#else
    m_win = glfwCreateWindow(TARGET_WIN_WIDTH, TARGET_WIN_HEIGHT, "Nesish",
                             NULL, NULL);
#endif
    if (!m_win) {
        goto l_err;
    }
    // Load OpenGL functions
    glfwMakeContextCurrent(m_win);
#ifndef SH_TGT_WEB
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        goto l_err;
    }
#endif
#ifndef SH_TGT_WEB
    // We do the timing ourselves.
    glfwSwapInterval(0);
#else
    // Setup this later on via emscripten APIs
#endif
    // Register callbacks
    glfwSetKeyCallback(m_win, Application::key_callback);

#ifdef SH_TGT_WEB
    /* init textures */
    if (!m_black_frm_tex.from_black_frame(16, 16)) {
        goto l_err;
    }
#endif

    /* Init IMGUI */
    {
        IMGUI_CHECKVERSION();
        m_imgui_ctx = ImGui::CreateContext();
        if (!m_imgui_ctx) {
            goto l_err;
        }
#ifndef SH_TGT_WEB
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
        if (!ImGui_ImplGlfw_InitForOpenGL(m_win, true)) {
            goto l_err;
        }
        m_imgui_glfw_inited = true;
        if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
            goto l_err;
        }
        m_imgui_opengl_inited = true;

#ifndef SH_TGT_WEB
        ImGui_ImplGlfw_SetCallbacksChainForAllWindows(true);
#else
        // Manually save/load imgui config
        ImGui::GetIO().IniFilename = nullptr;
#if defined(SH_TGT_WEB) && SH_SAVE_IMGUI_INI
        // load
        ImGui::LoadIniSettingsFromDisk(SH_IMGUI_INI_FILEPATH);
#endif
#endif
    }

    /* Setup emulator controller */
    SH_TRY
    {
        m_p1.user = new sh::Controller(m_win, m_p1_keys);
        ASM_CTRL(m_p1);
        nh_plug_ctrl(m_emu, NH_CTRL_P1, &m_p1);
        m_p2.user = new sh::Controller(m_win, m_p2_keys);
        ASM_CTRL(m_p2);
        nh_plug_ctrl(m_emu, NH_CTRL_P2, &m_p2);
    }
    SH_CATCH(const std::exception &)
    {
        goto l_err;
    }

    /* Create sub windows */
    SH_TRY
    {
        m_sub_wins.insert(
            {PPU_DEBUGGER_NAME,
             new PPUDebugger(PPU_DEBUGGER_NAME, m_emu, &m_messager)});
        m_sub_wins.insert({CUSTOM_KEY_NAME,
                           new CustomKey(CUSTOM_KEY_NAME, m_emu, &m_messager)});
    }
    SH_CATCH(const std::exception &)
    {
        goto l_err;
    }

#if !SH_NO_AUDIO
    /* Create audio objects */
    SH_TRY
    {
        m_audio_buf = new AudioBuffer();
        m_audio_data =
            new AudioData{0, to_AudioBuffer(m_audio_buf), m_logger, false};
        m_resampler = new Resampler();
#ifndef SH_TGT_WEB
        m_pcm_writer = new PCMWriter();
#endif
    }
    SH_CATCH(const std::exception &)
    {
        goto l_err;
    }

    /* Init audio */
    if (!audio_setup(AUDIO_SAMPLE_RATE, AUDIO_BUF_SIZE, audio_playback,
                     m_audio_data)) {
        goto l_err;
    }
#else
    (void)(audio_playback);
#endif

    /* Adjust window size and show it */
    {
        // Cache target framebuffer size, cos glfwSetWindowSize() changes OpenGL
        // viewport
        int width, height;
        glfwGetFramebufferSize(m_win, &width, &height);
#ifndef SH_TGT_WEB
        // Change window size considering menubar
        // already current
        // glfwMakeContextCurrent(m_win);
        int menu_bar_height = get_menubar_height();
        glfwSetWindowSize(m_win, TARGET_WIN_WIDTH,
                          TARGET_WIN_HEIGHT + menu_bar_height);
#endif
        // Restore framebuffer area excluding menubar
        glViewport(0, 0, width, height);

        glfwShowWindow(m_win);
    }

    /* Load config */
    {
#ifdef SH_TGT_MACOS
        m_sleepless = false;
        (void)load_single_bool(m_sleepless, CONFIG_SECTION_DEBUG,
                               CONFIG_KEY_SLEEPLESS);
#endif
        m_muted = SH_MUTED_DEF;
        (void)load_single_bool(m_muted, CONFIG_SECTION_DEBUG, CONFIG_KEY_MUTED);
    }

    return true;
l_err:
    release();
    return false;
}

void
Application::release()
{
    release_game(); // if any

#if !SH_NO_AUDIO
    audio_shutdown();

#ifndef SH_TGT_WEB
    if (m_pcm_writer) {
        delete m_pcm_writer;
        m_pcm_writer = nullptr;
    }
#endif
    if (m_resampler) {
        delete m_resampler;
        m_resampler = nullptr;
    }
    if (m_audio_data) {
        delete m_audio_data;
        m_audio_data = nullptr;
    }
    if (m_audio_buf) {
        delete to_AudioBuffer(m_audio_buf);
        m_audio_buf = nullptr;
    }
#endif

    for (auto it : m_sub_wins) {
        Window *win = it.second;
        delete win;
    }
    m_sub_wins.clear();

    if (m_p2.user) {
        // NH_VALID(m_emu)
        nh_unplug_ctrl(m_emu, NH_CTRL_P2);
        delete (sh::Controller *)(m_p2.user);
        m_p2.user = nullptr;
    }
    if (m_p1.user) {
        // NH_VALID(m_emu)
        nh_unplug_ctrl(m_emu, NH_CTRL_P1);
        delete (sh::Controller *)(m_p1.user);
        m_p1.user = nullptr;
    }

    if (m_imgui_opengl_inited) {
        ImGui_ImplOpenGL3_Shutdown();
        m_imgui_opengl_inited = false;
    }
    if (m_imgui_glfw_inited) {
        ImGui_ImplGlfw_Shutdown();
        m_imgui_glfw_inited = false;
    }
    if (m_imgui_ctx) {
        ImGui::DestroyContext(m_imgui_ctx);
        m_imgui_ctx = nullptr;
    }

    if (m_win) {
        glfwMakeContextCurrent(m_win);
#ifdef SH_TGT_WEB
        m_black_frm_tex.cleanup();
#endif
        m_frame_tex.cleanup();
        glfwMakeContextCurrent(nullptr);

        glfwMakeContextCurrent(nullptr);
        glfwDestroyWindow(m_win);
        m_win = nullptr;
    }

    if (m_glfw_inited) {
        glfwTerminate();
        m_glfw_inited = false;
    }

    if (NH_VALID(m_emu)) {
        nh_release_console(m_emu);
        m_emu = NH_NULL;
    }

    if (m_logger) {
        delete m_logger;
        m_logger = nullptr;
    }
}

int
Application::run()
{
    g_app = this;

    /* Main loop */
#ifdef SH_TGT_WEB
    // @NOTE: Better don't have objects with non-trivial destructor on the stack
    // before this call. Currently, this holds. See main() entry for details.
    //
    // https://emscripten.org/docs/api_reference/emscripten.h.html#c.emscripten_set_main_loop
    // Quote: "Currently, using the new Wasm exception handling and
    // simulate_infinite_loop == true at the same time does not work yet in C++
    // projects that have objects with destructors on the stack at the time of
    // the call."
    //
    // Although we don't enable exception handling, we adhere to that since we
    // can, to avoid potential problems.
#ifndef SH_EXPLICIT_RAF
    g_em_loop_cb = [this]() -> void {
        on_em_tick(this, emscripten_get_now());
    };
    g_em_loop_inited = false;
    emscripten_set_main_loop(em_loop_cb_c, 0, 1);
    g_em_loop_inited = false;
#else
    emscripten_request_animation_frame_loop(em_raf_cb, this);
#endif
#else
    auto currTime = std::chrono::steady_clock::now();
    auto nextLoopTime = currTime + std::chrono::duration<double>(0.0);
    while (true) {
        currTime = std::chrono::steady_clock::now();
        if (currTime >= nextLoopTime) {
            /* Handle inputs, process events */
            glfwPollEvents();
            /* Close window if necessary */
            if (glfwWindowShouldClose(m_win)) {
                goto l_loop_end;
            }

            /* Emulate & render */
            tick(FRAME_TIME);

            /* Update time */
            nextLoopTime = currTime + std::chrono::duration<double>(FRAME_TIME);
            // After testing, sleep implementation on MacOS (combined with
            // libc++) has higher resolution and reliability.
#ifdef SH_TGT_MACOS
            if (!m_sleepless) {
                std::this_thread::sleep_until(nextLoopTime);
            }
#endif
        }
    }
l_loop_end:;
#endif

#ifndef SH_EXPLICIT_RAF
    g_app = nullptr;
#endif
    return 0;
}

void
Application::tick(double i_delta_s)
{
    /* Emulate */
    if (running_game() && !m_paused) {
        NHCycle ticks = nh_advance(m_emu, i_delta_s);
        for (decltype(ticks) i = 0; i < ticks; ++i) {
#if SH_PROFILE_FRM
            std::chrono::steady_clock::time_point begin =
                std::chrono::steady_clock::now();
#endif
            int sample_avail = nh_tick(m_emu, nullptr);
#if SH_PROFILE_FRM
            std::chrono::steady_clock::time_point end =
                std::chrono::steady_clock::now();
            m_acc_time += std::chrono::duration<double>(end - begin).count();
            ++m_acc_ticks;
            if (m_acc_ticks >= 1789773 * FRAME_TIME) {
                SH_LOG_INFO(m_logger, "Time difference = {} [s]", m_acc_time);
                m_acc_time = 0;
                m_acc_ticks = 0;
            }
#endif
            if (sample_avail) {
#if !SH_NO_AUDIO
                double sample = m_muted ? 0.0 : nh_get_sample(m_emu);
                m_resampler->clock(short(sample * 32767));
                // Once clocked, samples must be drained to avoid
                // buffer overflow.
                short buf[AUDIO_BUF_SIZE];
                while (m_resampler->samples_avail(buf, AUDIO_BUF_SIZE)) {
                    for (decltype(AUDIO_BUF_SIZE) j = 0; j < AUDIO_BUF_SIZE;
                         ++j) {
                        // If failed, sample gets dropped, but we are free
                        // of inconsistent emulation due to blocking delay
                        if (!to_AudioBuffer(m_audio_buf)
                                 ->try_send(buf[j] / 32767.f)) {
#if DEBUG_AUDIO
                            SH_LOG_WARN(m_logger, "Sample gets dropped");
#endif
                        }

#ifndef SH_TGT_WEB
                        if (m_pcm_writer->is_open() && !m_muted) {
                            m_pcm_writer->write_s16le(buf[j]);
                        }
#endif
                    }
                }
#endif
            }
        }
    }

    /* Render */
    {
        glfwMakeContextCurrent(m_win);

#ifndef SH_TGT_WEB
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#else
        glClearColor(54 / 255.f, 134 / 255.f, 160 / 255.f, 1.0f);
#endif
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Emualtor content
#ifndef SH_TGT_WEB
        if (running_game()) {
            NHFrame framebuf = nh_get_frm(m_emu);
            m_renderer->render(framebuf);
#else
        if (ImGui::Begin("Frame", NULL,
                         ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_AlwaysAutoResize)) {
            if (!running_game()) {
                ImGui::Image(
                    reinterpret_cast<ImTextureID>(m_black_frm_tex.texture()),
                    ImVec2(TARGET_WIN_WIDTH, TARGET_WIN_HEIGHT), {0, 0}, {1, 1},
                    {1, 1, 1, 1}, {1, 1, 1, 1});
            } else {
                NHFrame framebuf = nh_get_frm(m_emu);
                if (m_frame_tex.from_frame(framebuf)) {
                    ImGui::Image(
                        reinterpret_cast<ImTextureID>(m_frame_tex.texture()),
                        ImVec2(TARGET_WIN_WIDTH, TARGET_WIN_HEIGHT), {0, 0},
                        {1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});
                } else {
                    ImGui::Text("Failed to update frame texture");
                }
            }
        }
        ImGui::End();
#endif
#ifndef SH_TGT_WEB
        }
#endif

        // Menubar
        draw_menubar();

        // Sub windows
        for (auto it : m_sub_wins) {
            Window *win = it.second;
            win->render();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        glfwSwapBuffers(m_win);
    }

    /* Save IMGUI config */
#if defined(SH_TGT_WEB) && SH_SAVE_IMGUI_INI
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantSaveIniSettings) {
        ImGui::SaveIniSettingsToDisk(SH_IMGUI_INI_FILEPATH);
        web_sync_fs_async(); // Initiate asynchronously
        io.WantSaveIniSettings = false;
    }
#endif
}

void
Application::load_game(const char *i_id_path, const char *i_real_path)
{
    /* Insert cartridge */
    NHErr nh_err = nh_insert_cartridge(m_emu, i_real_path);
    if (NH_FAILED(nh_err)) {
        goto l_err;
    }

#ifndef SH_TGT_WEB
    /* Create OpenGL renderer for emulator content */
    // need to call gl functions.
    glfwMakeContextCurrent(m_win);
    SH_TRY
    {
        m_renderer = new Renderer();
    }
    SH_CATCH(const std::exception &)
    {
        goto l_err;
    }
    if (m_renderer->setup()) {
        goto l_err;
    }
#endif

#if !SH_NO_AUDIO
    // resampler
    if (!m_resampler->init(AUDIO_BUF_SIZE * 2)) // More than enough
    {
        goto l_err;
    }
    if (!m_resampler->set_rates(double(nh_get_sample_rate(m_emu)),
                                AUDIO_SAMPLE_RATE)) {
        goto l_err;
    }

    // pcm recorder
#if DEBUG_AUDIO_PCM && !defined(SH_TGT_WEB)
    if (m_pcm_writer->open(nb::resolve_exe_dir("audio.pcm"))) {
        goto l_err;
    }
#endif
#endif

    /* Powerup */
    nh_power_up(m_emu);

    /* Flag running */
    SH_TRY
    {
        m_running_rom = i_id_path;
    }
    SH_CATCH(const std::exception &)
    {
        goto l_err;
    }

#if !SH_NO_AUDIO
#ifdef SH_USE_SOKOL_AUDIO
    /* Resume audio buffer */
    m_audio_data->stopped = false;
#else
    /* Start audio playback */
    if (!audio_start()) {
        goto l_err;
    }
#endif
#endif

    return;
l_err:
    release_game();
}

void
Application::release_game()
{
#if !SH_NO_AUDIO
#ifdef SH_USE_SOKOL_AUDIO
    m_audio_data->stopped = true;
#else
    audio_stop();
#endif
#endif

    m_running_rom.clear();

#if !SH_NO_AUDIO
#ifndef SH_TGT_WEB
    if (m_pcm_writer) {
        (void)m_pcm_writer->close();
    }
#endif
    if (m_resampler) {
        m_resampler->close();
    }
#endif

#ifndef SH_TGT_WEB
    if (m_renderer) {
        // m_win != nullptr;
        // need to call gl functions.
        glfwMakeContextCurrent(m_win);
        delete m_renderer;
        m_renderer = nullptr;
        glfwMakeContextCurrent(nullptr);
    }
#endif

    if (NH_VALID(m_emu)) {
        nh_remove_cartridge(m_emu);
    }
}

void
Application::on_game_opened(const char *i_id_path, const char *i_real_path)
{
    if (!running_game()) {
        load_game(i_id_path, i_real_path);
    } else if (m_running_rom != i_id_path) {
        release_game();
        load_game(i_id_path, i_real_path);
    }
}

bool
Application::running_game() const
{
    return !m_running_rom.empty();
}

int
Application::get_menubar_height()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    float menu_height = 0.0f;
    draw_menubar(&menu_height);
    // better larger than smaller
    menu_height = std::ceil(menu_height);

    ImGui::EndFrame();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    return (int)menu_height;
}

void
Application::draw_menubar(float *o_height)
{
#ifndef SH_TGT_WEB
    bool open_game_popup = false;
#endif
    static bool s_open_about = false;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open")) {
#ifndef SH_TGT_WEB
                open_game_popup = true;
#else
                js_open_game_popup();
#endif
            }
            if (ImGui::MenuItem("Stop")) {
                release_game();
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Control")) {
            if (ImGui::MenuItem(m_paused ? "Resume###PauseControl"
                                         : "Pause###PauseControl")) {
                m_paused = !m_paused;
            }
            if (ImGui::MenuItem("Power")) {
                if (running_game()) {
                    nh_power_up(m_emu);
                }
            }
            if (ImGui::MenuItem("Reset")) {
                if (running_game()) {
                    nh_reset(m_emu);
                }
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Option")) {
            if (ImGui::MenuItem(CUSTOM_KEY_NAME)) {
                m_sub_wins.at(CUSTOM_KEY_NAME)->show();
            }
#if !SH_NO_AUDIO
            if (ImGui::MenuItem("Mute", nullptr, m_muted)) {
                if (save_single_bool(!m_muted, CONFIG_SECTION_DEBUG,
                                     CONFIG_KEY_MUTED)) {
                    m_muted = !m_muted;
                }
            }
#endif

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem(PPU_DEBUGGER_NAME)) {
                m_sub_wins.at(PPU_DEBUGGER_NAME)->show();
            }
#ifdef SH_TGT_MACOS
            if (ImGui::BeginMenu("Switch")) {
                if (ImGui::MenuItem("Sleepless", nullptr, m_sleepless)) {
                    if (save_single_bool(!m_sleepless, CONFIG_SECTION_DEBUG,
                                         CONFIG_KEY_SLEEPLESS)) {
                        m_sleepless = !m_sleepless;
                    }
                }

                ImGui::EndMenu();
            }
#endif
            if (ImGui::BeginMenu("Log Level")) {
                constexpr NHLogLevel LOG_ITEMS[] = {
                    NH_LOG_OFF,  NH_LOG_FATAL, NH_LOG_ERROR, NH_LOG_WARN,
                    NH_LOG_INFO, NH_LOG_DEBUG, NH_LOG_TRACE,
                };
                for (decltype(sizeof(LOG_ITEMS)) i = 0;
                     i < sizeof(LOG_ITEMS) / sizeof(NHLogLevel); ++i) {
                    if (ImGui::MenuItem(log_level_to_name(LOG_ITEMS[i]),
                                        nullptr,
                                        LOG_ITEMS[i] == m_logger->level)) {
                        if (save_log_level(LOG_ITEMS[i])) {
                            m_logger->set_level(LOG_ITEMS[i]);
                            m_nh_logger.active = m_logger->level;
                        }
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("About")) {
            s_open_about = true;
        }

        if (o_height) {
            *o_height = ImGui::GetWindowSize().y;
        }
        ImGui::EndMainMenuBar();
    }

#ifndef SH_TGT_WEB
    if (open_game_popup) {
        ImGui::OpenPopup("Open Game");
    }
    if (m_file_dialog.showFileDialog(
            "Open Game", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
            ImVec2(0, 0), ".nes")) {
        on_game_opened(m_file_dialog.selected_path.c_str(),
                       m_file_dialog.selected_path.c_str());
    }
#endif

    if (s_open_about) {
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 35, 0),
                                 ImGuiCond_Once);
        if (ImGui::Begin("About", &s_open_about,
                         ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse)) {
            ImGui::Text("Nesish v0.1.0");
            ImGui::Indent();
            ImGui::Text("Presented by Snull, licensed under the MIT License");
            ImGui::Text("https://github.com/llunS/Nesish");
            ImGui::Unindent();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Development dependencies");
            ImGui::Indent();
            ImGui::Text("googletest");
            ImGui::Unindent();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Production dependencies");
            ImGui::Indent();
            ImGui::Text("spdlog");
            ImGui::Text("fmt");
            ImGui::Text("glfw");
            ImGui::Text("Dear ImGui");
            ImGui::Text("blip_buf, by Shay Green (blargg)");
            ImGui::Text("mINI, by pulzed");
#ifndef SH_TGT_WEB
            ImGui::Text("glad");
            ImGui::Text("ImGuiFileBrowser, by gallickgunner");
#endif
#ifndef SH_USE_SOKOL_AUDIO
            ImGui::Text("rtaudio");
#else
            ImGui::Text("sokol (audio & log), by floooh");
#endif
            ImGui::Unindent();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Acknowledgments");
            ImGui::Indent();
            ImGui::TextWrapped(
                "Sincere thanks to the NesDev community (including and not "
                "limited to the great Wiki and the Discord server), "
                "wouldn't have made it without your generous help.");
            ImGui::Unindent();
        }
        ImGui::End();
    }
}

void
error_callback(int error, const char *description)
{
    std::fprintf(stderr, "Error: %d, %s\n", error, description);
}

void
Application::key_callback(GLFWwindow *, int vkey, int, int action, int)
{
    // @FIXME: Effectively global, bad
    Application *app = g_app;
    if (app && GLFW_RELEASE == action) {
        auto it = app->m_sub_wins.find(CUSTOM_KEY_NAME);
        if (it != app->m_sub_wins.end()) {
            CustomKey *custom_key = static_cast<CustomKey *>(it->second);
            NHCtrlPort port;
            NHKey key;
            if (custom_key->on_key_released(vkey, port, key)) {
                if (NH_CTRL_P1 == port) {
                    app->m_p1_keys[key] = vkey;
                } else if (NH_CTRL_P2 == port) {
                    app->m_p2_keys[key] = vkey;
                }
            }
        }
    }
}

void
pv_log(NHLogLevel level, const char *msg, void *user)
{
    sh::Logger *logger = static_cast<sh::Logger *>(user);
    switch (level) {
    case NH_LOG_FATAL:
        SH_LOG_FATAL(logger, msg);
        break;
    case NH_LOG_ERROR:
        SH_LOG_ERROR(logger, msg);
        break;
    case NH_LOG_WARN:
        SH_LOG_WARN(logger, msg);
        break;
    case NH_LOG_INFO:
        SH_LOG_INFO(logger, msg);
        break;
    case NH_LOG_DEBUG:
        SH_LOG_DEBUG(logger, msg);
        break;
    case NH_LOG_TRACE:
        SH_LOG_TRACE(logger, msg);
        break;

    default:
        break;
    }
}

#ifndef SH_USE_SOKOL_AUDIO
int
audio_playback(void *output_buffer, void *, unsigned int num_frames, double,
               RtAudioStreamStatus status, void *user_data)
#else
void
audio_playback(float *output_buffer, int num_frames, int num_channels,
               void *user_data)
#endif
{
    AudioData *audio_data = (AudioData *)user_data;

#ifndef SH_USE_SOKOL_AUDIO
    (void)(status);
#if DEBUG_AUDIO
    if (status) {
        SH_LOG_WARN(audio_data->logger,
                    "Stream underflow for buffer of {} detected!", num_frames);
    }
#endif
#else
    (void)(num_channels); // num_channels == 2
#endif

    // Write audio data
    sample_t *buffer = (sample_t *)output_buffer;
    for (decltype(num_frames) i = 0; i < num_frames; ++i) {
        sample_t sample;
        if (audio_data->buf->try_receive(sample)) {
            audio_data->prev = sample;
        } else {
            if (audio_data->stopped) {
                sample = 0;
                audio_data->prev = sample;
            } else {
                sample = audio_data->prev;
            }
        }

        // interleaved, 2 channels, mono ouput
        for (unsigned int j = 0; j < 2; ++j) {
            *buffer++ = sample;
        }
    }
#ifndef SH_USE_SOKOL_AUDIO
    return 0;
#endif
}

void
pv_strobe(int enabled, void *user)
{
    sh::Controller *ctrl = static_cast<sh::Controller *>(user);
    ctrl->strobe(enabled);
}
int
pv_report(void *user)
{
    sh::Controller *ctrl = static_cast<sh::Controller *>(user);
    return ctrl->report();
}
void
pv_reset(void *user)
{
    sh::Controller *ctrl = static_cast<sh::Controller *>(user);
    ctrl->reset();
}

} // namespace sh

#ifdef SH_TGT_WEB

void
on_em_tick(sh::Application *app, double now)
{
    if (!g_em_loop_inited) {
        g_em_loop_time = now;
        g_em_loop_inited = true;
    } else {
        double delta = (now - g_em_loop_time) / 1000.0;
        // For now we have to clamp large delta, for we will be otherwise
        // spending our lifetime to catch up with the large amount of work (to
        // user this means freezing).
        // Large delta time happens:
        // 1) Open up chrome devtools,
        // 2) Switch to other tabs,
        // We need to improve the performance of the emulator to fix this.
        if (delta > FRAME_TIME) {
            delta = FRAME_TIME;
        }
        app->tick(delta);
        g_em_loop_time = now;
    }
}

void
nh_on_canvas_size_changed(int width, int height)
{
    if (g_app) {
        if (g_app->m_win) {
            glfwSetWindowSize(g_app->m_win, width, height);
        }
    }
}

void
nh_on_game_opened(const char *basename)
{
    // e.g. XXX.nes
    // So file in different directories but with the same name as the current
    // one gets ignored.
    // So far so good.
    if (g_app) {
        g_app->on_game_opened(basename, "/tmp_open_game");
    }
}

#endif
