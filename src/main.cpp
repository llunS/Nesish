#include "CLI/CLI.hpp"
#include "common/logger.hpp"
#include "glfw_app/app.hpp"

#include <string>

int
main(int argc, char **argv)
{
    // -- cli options
    spdlog::level::level_enum log_level = LN_DEFAULT_LOG_LEVEL;
    std::string rom_path;
    {
        CLI::App app;

        app.add_option("-l", log_level, "Set log level, 0(max) - 6(off)")
            ->option_text("\x7F")
            ->check(CLI::Range(spdlog::level::trace, spdlog::level::off));

        app.add_option("<rompath>", rom_path, "Rom path")
            ->option_text("\x7F")
            ->required(true);

        CLI11_PARSE(app, argc, argv);
    }

    ln::init_logger(log_level);

    ln_app::App app;
    return app.run(rom_path);
}
