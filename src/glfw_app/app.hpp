
#pragma once

#include <string>

#include "glfw_app/dllexport.h"

namespace ln_app {

struct LN_APP_API App {
    int
    run(const std::string &i_rom_path);
};

} // namespace ln_app
