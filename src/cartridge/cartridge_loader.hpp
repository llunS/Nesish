#ifndef LN_CARTRIDGE_CARTRIDGELOADER_HPP
#define LN_CARTRIDGE_CARTRIDGELOADER_HPP

#include "common/error.hpp"
#include <string>
#include "cartridge/cartridge.hpp"
#include "cartridge/cartridge_type.hpp"

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
    pvt_load_ines_header(std::FILE *i_file, INES *io_ines);
    static Error
    pvt_load_ines_trainer(std::FILE *i_file, INES *io_ines);
    static Error
    pvt_load_ines_prg_rom(std::FILE *i_file, INES *io_ines);
    static Error
    pvt_load_ines_chr_rom(std::FILE *i_file, INES *io_ines);
};

} // namespace ln

#endif // LN_CARTRIDGE_CARTRIDGELOADER_HPP