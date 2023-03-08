#pragma once

#include "common/error.hpp"
#include <string>
#include "console/cartridge/cartridge.hpp"
#include "console/cartridge/cartridge_type.hpp"

namespace ln {

struct INES;

struct CartridgeLoader {
  public:
    static Error
    load_cartridge(const std::string &i_rom_path, CartridgeType i_type,
                   Cartridge **o_cartridge);

  private:
    static Error
    load_ines(const std::string &i_rom_path, Cartridge **o_cartridge);

    static Error
    pv_load_ines_header(std::FILE *i_file, INES *io_ines);
    static Error
    pv_load_ines_trainer(std::FILE *i_file, INES *io_ines);
    static Error
    pv_load_ines_prg_rom(std::FILE *i_file, INES *io_ines);
    static Error
    pv_load_ines_chr_rom(std::FILE *i_file, INES *io_ines);
};

} // namespace ln
