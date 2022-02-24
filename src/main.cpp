#include "CLI/CLI.hpp"
#include "common/logger.hpp"
#include "console/emulator.hpp"

int
main(int argc, char **argv)
{
    // -- cli options
    spdlog::level::level_enum log_level = LN_DEFAULT_LOG_LEVEL;
    {
        CLI::App app;

        app.add_option("-l,--level", log_level,
                       "Set log level in [0, 6] with 6 being off")
            ->option_text("\x7F")
            ->check(CLI::Range(spdlog::level::trace, spdlog::level::off));

        CLI11_PARSE(app, argc, argv);
    }

    ln::init_logger(log_level);

    ln::Emulator emulator;
    emulator.power_up();
    return 0;
}
