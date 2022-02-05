#include "console/emulator.hpp"

int
main(int, char **)
{
    ln::Emulator emulator;
    auto err = emulator.power_up();
    return LN_FAILED(err);
}
