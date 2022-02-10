#ifndef LN_CARTRIDGE_INES_HPP
#define LN_CARTRIDGE_INES_HPP

#include "cartridge/cartridge.hpp"
#include "common/klass.hpp"

#include <cstdint>

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
        uint8_t nes_magic[4];
        uint8_t prg_rom_size; // in 16KB units.
        uint8_t chr_rom_size; // in 8KB units, 0 implies CHR RAM.

        // flag 6
        uint8_t mirror : 1; // 0: horizontal, 1: vertical.
        uint8_t persistent_memory : 1;
        uint8_t trainer : 1; // ignored, unsupported.
        uint8_t four_screen_vram : 1;
        uint8_t mapper_lower : 4;

        // flag 7
        uint8_t vs_unisystem : 1;  // ignored, unsupported.
        uint8_t playchoice_10 : 1; // ignored, unsupported.
        uint8_t ines2 : 2;         // 2: NES 2.0 format.
        uint8_t mapper_upper : 4;

        uint8_t prg_ram_size; // in 8KB units, 0 infers 8KB for compatibility.
    } m_header;
    uint8_t *m_prg_rom;
    uint8_t *m_chr_rom;

    uint8_t m_mapper_number;
};

} // namespace ln

#endif // LN_CARTRIDGE_INES_HPP
