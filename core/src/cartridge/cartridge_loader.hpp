#pragma once

#include <string>
#include "cartridge/cartridge.hpp"
#include "cartridge/cartridge_type.hpp"

struct NHLogger;

namespace nh {

struct INES;

struct CartridgeLoader {
  public:
    static NHErr
    load_cartridge(const std::string &i_rom_path, CartridgeType i_type,
                   Cartridge **o_cartridge, NHLogger *i_logger);

  private:
    static NHErr
    load_ines(const std::string &i_rom_path, Cartridge **o_cartridge,
              NHLogger *i_logger);

    static NHErr
    pv_load_ines_header(std::FILE *i_file, INES *io_ines, NHLogger *i_logger);
    static NHErr
    pv_load_ines_trainer(std::FILE *i_file, INES *io_ines, NHLogger *i_logger);
    static NHErr
    pv_load_ines_prg_rom(std::FILE *i_file, INES *io_ines, NHLogger *i_logger);
    static NHErr
    pv_load_ines_chr_rom(std::FILE *i_file, INES *io_ines, NHLogger *i_logger);
};

} // namespace nh
