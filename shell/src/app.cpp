#include "app.hpp"

#include "gui/application.hpp"

namespace sh {

int
run(AppOpt i_opts, Logger *i_logger)
{
    Application app;
    if (!app.init(i_opts, i_logger))
    {
        return 1;
    }

    int err = app.run();

    app.release();
    return err;
}

} // namespace sh
