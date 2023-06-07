#include "application.hpp"

#include <cstdio>
#include <chrono>
#ifdef SH_TGT_MACOS
#include <thread>
#endif
#include <cmath>

#include "audio/resampler.hpp"
#include "audio/pcm_writer.hpp"
#include "audio/channel.hpp"
#ifndef SH_USE_SOKOL_AUDIO
#include "audio/backend_rtaudio.hpp"
#else
#include "audio/backend_sokol.hpp"
#endif

#include "nhbase/path.hpp"

#include "glad.h"
#include "rendering/renderer.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "misc/config.hpp"

#include "gui/ppu_debugger.hpp"
#include "gui/custom_key.hpp"

namespace sh {

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

typedef float sample_t;
typedef Channel<sample_t, AUDIO_CH_SIZE> AudioBuffer;

#define to_AudioBuffer(ptr) (static_cast<AudioBuffer *>(ptr))

struct AudioData {
    sample_t prev;
    AudioBuffer *buf;
    Logger *logger;
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

Application *Application::s_instance = nullptr;

Application::Application()
    : m_win(nullptr)
    , m_glfw_inited(false)
    , m_imgui_ctx(nullptr)
    , m_imgui_glfw_inited(false)
    , m_imgui_opengl_inited(false)
    , m_logger(nullptr)
    , m_emu(NH_NULL)
    , m_renderer(nullptr)
    , m_audio_buf(nullptr)
    , m_audio_data(nullptr)
    , m_resampler(nullptr)
    , m_pcm_writer(nullptr)
    , m_paused(false)
    , m_sleepless(false)
    , m_messager(this)
{
    m_p1.user = nullptr;
    m_p2.user = nullptr;
}

Application::~Application() {}

bool
Application::init(Logger *i_logger)
{
    if (!i_logger)
    {
        goto l_err;
    }
    m_logger = i_logger;

    /* load key config */
    if (!load_key_config(m_p1_keys, m_p2_keys, m_logger))
    {
        goto l_err;
    }

    /* create emulator instance */
    m_nh_logger.log = pv_log;
    m_nh_logger.user = m_logger;
    m_nh_logger.active = m_logger->level;
    m_emu = nh_new_console(&m_nh_logger);
    if (!NH_VALID(m_emu))
    {
        goto l_err;
    }

    /* init glfw */
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        goto l_err;
    }
    m_glfw_inited = true;

    /* create glfw window */
    // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);         // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true); // Required on Mac
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);
    // hide first, cos we are about to adjust its size
    glfwWindowHint(GLFW_VISIBLE, false);
    m_win = glfwCreateWindow(TARGET_WIN_WIDTH, TARGET_WIN_HEIGHT, "Nesish",
                             NULL, NULL);
    if (!m_win)
    {
        goto l_err;
    }
    // Load OpenGL functions
    glfwMakeContextCurrent(m_win);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        goto l_err;
    }
    // We do the timing ourselves.
    glfwSwapInterval(0);
    // Register callbacks
    glfwSetKeyCallback(m_win, Application::key_callback);

    /* Init IMGUI */
    {
        IMGUI_CHECKVERSION();
        m_imgui_ctx = ImGui::CreateContext();
        if (!m_imgui_ctx)
        {
            goto l_err;
        }
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        if (!ImGui_ImplGlfw_InitForOpenGL(m_win, true))
        {
            goto l_err;
        }
        m_imgui_glfw_inited = true;
        const char *glsl_version = "#version 330";
        if (!ImGui_ImplOpenGL3_Init(glsl_version))
        {
            goto l_err;
        }
        m_imgui_opengl_inited = true;

        ImGui_ImplGlfw_SetCallbacksChainForAllWindows(true);
    }

    /* Setup emulator controller */
    try
    {
        m_p1.user = new sh::Controller(m_win, m_p1_keys);
        ASM_CTRL(m_p1);
        nh_plug_ctrl(m_emu, NH_CTRL_P1, &m_p1);
        m_p2.user = new sh::Controller(m_win, m_p2_keys);
        ASM_CTRL(m_p2);
        nh_plug_ctrl(m_emu, NH_CTRL_P2, &m_p2);
    }
    catch (const std::exception &)
    {
        goto l_err;
    }

    /* Create sub windows */
    try
    {
        m_sub_wins.insert(
            {PPU_DEBUGGER_NAME,
             new PPUDebugger(PPU_DEBUGGER_NAME, m_emu, &m_messager)});
        m_sub_wins.insert({CUSTOM_KEY_NAME,
                           new CustomKey(CUSTOM_KEY_NAME, m_emu, &m_messager)});
    }
    catch (const std::exception &)
    {
        goto l_err;
    }

    /* Adjust window size and show it */
    {
        // Cache target framebuffer size, cos glfwSetWindowSize() changes OpenGL
        // viewport
        int width, height;
        glfwGetFramebufferSize(m_win, &width, &height);
        // Change window size considering menubar
        // already current
        // glfwMakeContextCurrent(m_win);
        int menu_bar_height = get_menubar_height();
        glfwSetWindowSize(m_win, TARGET_WIN_WIDTH,
                          TARGET_WIN_HEIGHT + menu_bar_height);
        // Restore framebuffer area excluding menubar
        glViewport(0, 0, width, height);

        glfwShowWindow(m_win);
    }

    /* Load config */
    {
        m_sleepless = false;
        (void)load_sleepless(m_sleepless);
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

    for (auto it : m_sub_wins)
    {
        Window *win = it.second;
        delete win;
    }
    m_sub_wins.clear();

    if (m_p2.user)
    {
        // NH_VALID(m_emu)
        nh_unplug_ctrl(m_emu, NH_CTRL_P2);
        delete (sh::Controller *)(m_p2.user);
        m_p2.user = nullptr;
    }
    if (m_p1.user)
    {
        // NH_VALID(m_emu)
        nh_unplug_ctrl(m_emu, NH_CTRL_P1);
        delete (sh::Controller *)(m_p1.user);
        m_p1.user = nullptr;
    }

    if (m_imgui_opengl_inited)
    {
        ImGui_ImplOpenGL3_Shutdown();
        m_imgui_opengl_inited = false;
    }
    if (m_imgui_glfw_inited)
    {
        ImGui_ImplGlfw_Shutdown();
        m_imgui_glfw_inited = false;
    }
    if (m_imgui_ctx)
    {
        ImGui::DestroyContext(m_imgui_ctx);
        m_imgui_ctx = nullptr;
    }

    if (m_win)
    {
        glfwDestroyWindow(m_win);
        m_win = nullptr;
    }

    if (m_glfw_inited)
    {
        glfwTerminate();
        m_glfw_inited = false;
    }

    if (NH_VALID(m_emu))
    {
        nh_release_console(m_emu);
        m_emu = NH_NULL;
    }

    m_logger = nullptr;
}

int
Application::run()
{
    Application::s_instance = this;

    /* Main loop */
    auto currTime = std::chrono::steady_clock::now();
    auto nextLoopTime = currTime + std::chrono::duration<double>(0.0);
    while (true)
    {
        currTime = std::chrono::steady_clock::now();
        if (currTime >= nextLoopTime)
        {
            /* Handle inputs, process events */
            glfwPollEvents();
            /* Close window if necessary */
            if (glfwWindowShouldClose(m_win))
            {
                goto l_loop_end;
            }

            /* Emulate & render */
            tick();

            /* Update time */
            nextLoopTime = currTime + std::chrono::duration<double>(FRAME_TIME);
            // After testing, sleep implementation on MacOS (combined with
            // libc++) has higher resolution and reliability.
#ifdef SH_TGT_MACOS
            if (!m_sleepless)
            {
                std::this_thread::sleep_until(nextLoopTime);
            }
#endif
        }
    }
l_loop_end:;

    Application::s_instance = nullptr;
    return 0;
}

void
Application::tick()
{
    /* Emulate */
    if (running_game() && !m_paused)
    {
        NHCycle ticks = nh_advance(m_emu, FRAME_TIME);
        for (decltype(ticks) i = 0; i < ticks; ++i)
        {
            if (nh_tick(m_emu, nullptr))
            {
                double sample = nh_get_sample(m_emu);
                m_resampler->clock(short(sample * 32767));
                // Once clocked, samples must be drained to avoid
                // buffer overflow.
                short buf[AUDIO_BUF_SIZE];
                while (m_resampler->samples_avail(buf, AUDIO_BUF_SIZE))
                {
                    for (decltype(AUDIO_BUF_SIZE) j = 0; j < AUDIO_BUF_SIZE;
                         ++j)
                    {
                        // If failed, sample gets dropped, but we are free
                        // of inconsistent emulation due to blocking delay
                        if (!to_AudioBuffer(m_audio_buf)
                                 ->try_send(buf[j] / 32767.f))
                        {
#if DEBUG_AUDIO
                            SH_LOG_WARN(m_logger, "Sample gets dropped");
#endif
                        }

                        if (m_pcm_writer->is_open())
                        {
                            m_pcm_writer->write_s16le(buf[j]);
                        }
                    }
                }
            }
        }
    }

    /* Render */
    {
        glfwMakeContextCurrent(m_win);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Emualtor content
        if (running_game())
        {
            NHFrame framebuf = nh_get_frm(m_emu);
            m_renderer->render(framebuf);
        }

        // IMGUI
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Menubar
            draw_menubar();

            // Sub windows
            for (auto it : m_sub_wins)
            {
                Window *win = it.second;
                win->render();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
        }

        glfwSwapBuffers(m_win);
    }
}

void
Application::load_game(const std::string &i_rom_path)
{
    /* Insert cartridge */
    NHErr nh_err = nh_insert_cartridge(m_emu, i_rom_path.c_str());
    if (NH_FAILED(nh_err))
    {
        goto l_err;
    }

    /* Create OpenGL renderer for emulator content */
    // need to call gl functions.
    glfwMakeContextCurrent(m_win);
    try
    {
        m_renderer = new Renderer();
    }
    catch (const std::exception &)
    {
        goto l_err;
    }
    if (m_renderer->setup())
    {
        goto l_err;
    }

    /* Create audio objects */
    try
    {
        m_audio_buf = new AudioBuffer();
        m_audio_data = new AudioData{0, to_AudioBuffer(m_audio_buf), m_logger};
        m_resampler = new Resampler{AUDIO_BUF_SIZE * 2}; // More than enough
        m_pcm_writer = new PCMWriter();
    }
    catch (const std::exception &)
    {
        goto l_err;
    }

    /* init audio */
    // resampler
    if (!m_resampler->set_rates(double(nh_get_sample_rate(m_emu)),
                                AUDIO_SAMPLE_RATE))
    {
        goto l_err;
    }
#if DEBUG_AUDIO_PCM
    // pcm recorder
    if (m_pcm_writer->open(nb::path_join_exe("audio.pcm")))
    {
        goto l_err;
    }
#endif

    /* Powerup */
    nh_power_up(m_emu);

    /* Flag running */
    try
    {
        m_running_rom = i_rom_path;
    }
    catch (const std::exception &)
    {
        goto l_err;
    }

    /* Start audio playback */
    if (!audio_start(AUDIO_SAMPLE_RATE, AUDIO_BUF_SIZE, audio_playback,
                     m_audio_data))
    {
        goto l_err;
    }

    return;
l_err:
    release_game();
}

void
Application::release_game()
{
    (void)audio_stop();

    m_running_rom.clear();

    if (m_pcm_writer)
    {
        (void)m_pcm_writer->close();
    }
    if (m_resampler)
    {
        m_resampler->close();
    }

    if (m_pcm_writer)
    {
        delete m_pcm_writer;
        m_pcm_writer = nullptr;
    }
    if (m_resampler)
    {
        delete m_resampler;
        m_resampler = nullptr;
    }
    if (m_audio_data)
    {
        delete m_audio_data;
        m_audio_data = nullptr;
    }
    if (m_audio_buf)
    {
        delete to_AudioBuffer(m_audio_buf);
        m_audio_buf = nullptr;
    }

    if (m_renderer)
    {
        // m_win != nullptr;
        // need to call gl functions.
        glfwMakeContextCurrent(m_win);
        delete m_renderer;
        m_renderer = nullptr;
        glfwMakeContextCurrent(nullptr);
    }

    if (NH_VALID(m_emu))
    {
        nh_remove_cartridge(m_emu);
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

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    return (int)menu_height;
}

void
Application::draw_menubar(float *o_height)
{
    bool open_game_popup = false;
    static bool s_open_about = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
                open_game_popup = true;
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Control"))
        {
            if (ImGui::MenuItem(m_paused ? "Resume###PauseControl"
                                         : "Pause###PauseControl"))
            {
                m_paused = !m_paused;
            }
            if (ImGui::MenuItem("Power"))
            {
                if (running_game())
                {
                    nh_power_up(m_emu);
                }
            }
            if (ImGui::MenuItem("Reset"))
            {
                if (running_game())
                {
                    nh_reset(m_emu);
                }
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Option"))
        {
            if (ImGui::MenuItem(CUSTOM_KEY_NAME))
            {
                m_sub_wins.at(CUSTOM_KEY_NAME)->show();
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug"))
        {
            if (ImGui::MenuItem(PPU_DEBUGGER_NAME))
            {
                m_sub_wins.at(PPU_DEBUGGER_NAME)->show();
            }
            if (ImGui::BeginMenu("Switch"))
            {
                if (ImGui::MenuItem("Sleepless", nullptr, m_sleepless))
                {
                    if (save_sleepless(!m_sleepless))
                    {
                        m_sleepless = !m_sleepless;
                    }
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Log Level"))
            {
                constexpr NHLogLevel LOG_ITEMS[] = {
                    NH_LOG_OFF,  NH_LOG_FATAL, NH_LOG_ERROR, NH_LOG_WARN,
                    NH_LOG_INFO, NH_LOG_DEBUG, NH_LOG_TRACE,
                };
                for (decltype(sizeof(LOG_ITEMS)) i = 0;
                     i < sizeof(LOG_ITEMS) / sizeof(NHLogLevel); ++i)
                {
                    if (ImGui::MenuItem(log_level_to_name(LOG_ITEMS[i]),
                                        nullptr,
                                        LOG_ITEMS[i] == m_logger->level))
                    {
                        if (save_log_level(LOG_ITEMS[i]))
                        {
                            m_logger->set_level(LOG_ITEMS[i]);
                            m_nh_logger.active = m_logger->level;
                        }
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("About"))
        {
            s_open_about = true;
        }

        if (o_height)
        {
            *o_height = ImGui::GetWindowSize().y;
        }
        ImGui::EndMainMenuBar();
    }

    if (open_game_popup)
    {
        ImGui::OpenPopup("Open Game");
    }
    if (m_file_dialog.showFileDialog(
            "Open Game", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
            ImVec2(0, 0), ".nes"))
    {
        if (!running_game())
        {
            load_game(m_file_dialog.selected_path);
        }
        else if (m_running_rom != m_file_dialog.selected_path)
        {
            release_game();
            load_game(m_file_dialog.selected_path);
        }
    }

    if (s_open_about)
    {
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 35, 0),
                                 ImGuiCond_Once);
        if (ImGui::Begin("About", &s_open_about,
                         ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse))
        {
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
            ImGui::Text("fmt");
            ImGui::Text("spdlog");
            ImGui::Text("glfw");
            ImGui::Text("Dear ImGui");
            ImGui::Text("rtaudio");
            ImGui::Text("blip_buf, by Shay Green (blargg)");
            ImGui::Text("glad");
            ImGui::Text("ImGuiFileBrowser, by gallickgunner");
            ImGui::Text("mINI, by pulzed");
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
    fprintf(stderr, "Error: %d, %s\n", error, description);
}

void
Application::key_callback(GLFWwindow *, int vkey, int, int action, int)
{
    // @FIXME: Effectively global, bad
    Application *app = Application::s_instance;
    if (app && GLFW_RELEASE == action)
    {
        auto it = app->m_sub_wins.find(CUSTOM_KEY_NAME);
        if (it != app->m_sub_wins.end())
        {
            CustomKey *custom_key = static_cast<CustomKey *>(it->second);
            NHCtrlPort port;
            NHKey key;
            if (custom_key->on_key_released(vkey, port, key))
            {
                if (NH_CTRL_P1 == port)
                {
                    app->m_p1_keys[key] = vkey;
                }
                else if (NH_CTRL_P2 == port)
                {
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
    switch (level)
    {
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
    if (status)
    {
        SH_LOG_WARN(audio_data->logger,
                    "Stream underflow for buffer of {} detected!", num_frames);
    }
#endif
#else
    (void)(num_channels); // num_channels == 2
#endif

    // Write audio data
    sample_t *buffer = (sample_t *)output_buffer;
    for (decltype(num_frames) i = 0; i < num_frames; ++i)
    {
        sample_t sample;
        if (audio_data->buf->try_receive(sample))
        {
            audio_data->prev = sample;
        }
        else
        {
            sample = audio_data->prev;
        }

        // interleaved, 2 channels, mono ouput
        for (unsigned int j = 0; j < 2; ++j)
        {
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
