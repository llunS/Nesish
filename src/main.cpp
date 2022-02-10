#include "common/logger.hpp"
#include "console/emulator.hpp"

int
main(int, char **)
{
    ln::init_logger();

    ln::Emulator emulator;
    auto err = emulator.power_up();
    return LN_FAILED(err);
}
