#include "misc/logger.hpp"
#include "misc/config.hpp"
#include "gui/application.hpp"

int
main(int, char **)
{
    try
    {
        NHLogLevel log_level = SH_DEFAULT_LOG_LEVEL;
        (void)sh::load_log_level(log_level);
        sh::Logger logger(log_level);

        sh::Application app;
        if (!app.init(&logger))
        {
            return 1;
        }

        int err = app.run();

        app.release();

        return err;
    }
    catch (const std::exception &)
    {
        return 1;
    }
}
