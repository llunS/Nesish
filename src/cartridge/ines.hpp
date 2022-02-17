#ifndef LN_CARTRIDGE_INES_HPP
#define LN_CARTRIDGE_INES_HPP

#include "cartridge/cartridge.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"

namespace ln {

struct INES : public Cartridge {
  public:
    LN_KLZ_DELETE_COPY_MOVE(INES);
    ~INES();

    Error
    validate() override;

  public:
    friend struct CartridgeLoader;

  private:
    INES();

    void
    resolve();

  private:
    // https://wiki.nesdev.org/w/index.php/INES
    struct {
        Byte nes_magic[4];
        Byte prg_rom_size; // in 16KB units.
        Byte chr_rom_size; // in 8KB units, 0 implies CHR RAM.

        // flag 6
        Byte mirror : 1; // 0: horizontal, 1: vertical.
        Byte persistent_memory : 1;
        Byte trainer : 1; // ignored, unsupported.
        Byte four_screen_vram : 1;
        Byte mapper_lower : 4;

        // flag 7
        Byte vs_unisystem : 1;  // ignored, unsupported.
        Byte playchoice_10 : 1; // ignored, unsupported.
        Byte ines2 : 2;         // 2: NES 2.0 format.
        Byte mapper_higher : 4;

        Byte prg_ram_size; // in 8KB units, 0 infers 8KB for compatibility.
    } m_header;
    Byte *m_prg_rom;
    Byte *m_chr_rom;

    Byte m_mapper_number;
};

} // namespace ln

#endif // LN_CARTRIDGE_INES_HPP
