#pragma once

#include <memory>
#include <cstddef>

#include "console/cartridge/cartridge.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"

namespace ln {

struct Mapper;

struct INES : public Cartridge {
  public:
    LN_KLZ_DELETE_COPY_MOVE(INES);
    ~INES();

    Error
    validate() const override;

    void
    map_memory(Memory *o_memory, VideoMemory *o_video_memory) override;
    void
    unmap_memory(Memory *o_memory, VideoMemory *o_video_memory) override;

  public:
    struct RomAccessor {
      public:
        LN_KLZ_DELETE_COPY_MOVE(RomAccessor);

      public:
        bool
        h_mirror() const;

        void
        get_prg_rom(Byte **o_addr, std::size_t *o_size) const;
        void
        get_chr_rom(Byte **o_addr, std::size_t *o_size) const;
        bool
        use_chr_ram() const;

      private:
        RomAccessor(const INES *i_nes);
        friend struct INES; // allow constructor access

      private:
        const INES *m_ines;
    };
    friend struct RomAccessor;

  public:
    friend struct CartridgeLoader;

  private:
    INES();

    Error
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

        // This was a later extension to the iNES format and not widely used.
        Byte prg_ram_size; // in 8KB units, 0 infers 8KB for compatibility.
    } m_header;

  private:
    Byte m_mapper_number;
    std::unique_ptr<Mapper> m_mapper;

    Byte *m_prg_rom;
    std::size_t m_prg_rom_size;
    Byte *m_chr_rom;
    std::size_t m_chr_rom_size;
    bool m_use_chr_ram;

    RomAccessor m_rom_accessor;
};

} // namespace ln
