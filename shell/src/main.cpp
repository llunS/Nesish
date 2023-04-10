#include "CLI/CLI.hpp"

#include "misc/logger.hpp"
#include "app.hpp"

#include <string>
#include <memory>

int
main(int argc, char **argv)
{
    // -- cli options
    std::string rom_path;
    NHLogLevel log_level = SH_DEFAULT_LOG_LEVEL;
    bool debug_win = false;
    bool log_pcm = false;
    bool no_sleep = false;

    CLI::App app;
    app.add_option("<rompath>", rom_path, "Rom path")->required(true);
    app.add_flag("-d", debug_win, "With debug window");
    app.add_option("-l", log_level, "Log level, 0(off) - 6")
        ->option_text("<level>")
        ->check(CLI::Range(NH_LOG_OFF, NH_LOG_TRACE));
    app.add_flag("--pcm", log_pcm,
                 "Record audio PCM file in mono signed 16-bit integer format");
    app.add_flag("--no-sleep", no_sleep, "Disable experimental battery saver");
    CLI11_PARSE(app, argc, argv);

    std::unique_ptr<sh::Logger> logger{sh::Logger::create(log_level)};

    sh::AppOpt app_opts = sh::OPT_NONE;
    if (debug_win)
    {
        app_opts |= sh::OPT_DEBUG;
    }
    if (log_pcm)
    {
        app_opts |= sh::OPT_PCM;
    }
    if (no_sleep)
    {
        app_opts |= sh::OPT_NOSLEEP;
    }

    return sh::run(rom_path, app_opts, logger.get());
}
