#include "CLI/CLI.hpp"
#include "logger.hpp"
#include "app.hpp"

#include <string>

int
main(int argc, char **argv)
{
    // -- cli options
    std::string rom_path;
    NHLogLevel log_level = SH_DEFAULT_LOG_LEVEL;
    bool debug_win = false;
    bool log_pcm = false;
    bool audio_enabled = false;
    {
        CLI::App app;

        app.add_option("<rompath>", rom_path, "Rom path")
            ->option_text("\x7F")
            ->required(true);

        app.add_option("-l", log_level, "Log level, 0 - 6(off)")
            ->option_text("<level>")
            ->check(CLI::Range(NH_LOG_OFF, NH_LOG_TRACE));

        app.add_flag("-d", debug_win, "With debug window")->option_text("\x7F");

        app.add_flag("--pcm", log_pcm,
                     "Log audio PCM file in signed 16-bit integer format")
            ->option_text("\x7F");

        app.add_flag("-a", audio_enabled, "Experimental audio support")
            ->option_text("\x7F");

        CLI11_PARSE(app, argc, argv);
    }

    sh::init_logger(log_level);

    sh::AppOpt app_opts = sh::OPT_NONE;
    if (debug_win)
    {
        app_opts |= sh::OPT_DEBUG_WIN;
    }
    if (log_pcm)
    {
        app_opts |= sh::OPT_PCM;
    }
    if (audio_enabled)
    {
        app_opts |= sh::OPT_AUDIO;
    }

    return sh::run_app(rom_path, app_opts);
}
