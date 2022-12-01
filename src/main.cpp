#include "CLI/CLI.hpp"
#include "common/logger.hpp"
#include "glfw_app/app.hpp"

#include <string>

int
main(int argc, char **argv)
{
    // -- cli options
    std::string rom_path;
    spdlog::level::level_enum log_level = LN_DEFAULT_LOG_LEVEL;
    bool log_pcm = false;
    {
        CLI::App app;

        app.add_option("<rompath>", rom_path, "Rom path")
            ->option_text("\x7F")
            ->required(true);

        app.add_option("-l", log_level, "Set log level, 0 - 6(off)")
            ->option_text("<level>")
            ->check(CLI::Range(spdlog::level::trace, spdlog::level::off));

        app.add_flag("--pcm", log_pcm,
                     "Log audio PCM file in signed 16-bit integer format")
            ->option_text("\x7F");

        CLI11_PARSE(app, argc, argv);
    }

    ln::init_logger(log_level);

    ln_app::AppOpt app_opts = ln_app::OPT_NONE;
    if (log_pcm)
    {
        app_opts |= ln_app::OPT_PCM;
    }

    return ln_app::run_app(rom_path, app_opts);
}
