#pragma once

// Resolution
#define LN_NES_WIDTH 256
#define LN_NES_HEIGHT 240

#define LN_CPU_HZ 1789773

#define LN_ADDRESSABLE_SIZE (64 * 1024) // 16bit addressable memory

#define LN_INTERNAL_RAM_SIZE (2 * 1024) // 2KiB
#define LN_INTERNAL_RAM_ADDR_HEAD 0x0000
#define LN_INTERNAL_RAM_ADDR_TAIL 0x07FF
#define LN_INTERNAL_RAM_MASK 0x07FF // leftmost 5 bits zero

#define LN_RAM_ADDR_HEAD 0x0000
#define LN_RAM_ADDR_TAIL 0x1FFF
#define LN_RAM_MASK 0x1FFF // leftmost 3 bits zero

#define LN_PPU_ADDRESSABLE_SIZE (16 * 1024) // 14bit addressable memory
#define LN_PPU_ADDR_MASK 0x3FFF
#define LN_PPU_INTERNAL_RAM_SIZE (2 * 1024) // 2KiB
#define LN_OAM_SIZE 256                     // 256 bytes
#define LN_PALETTE_SIZE 32

#define LN_PPUCTRL_ADDR 0x2000
#define LN_PPUDATA_ADDR 0x2007
#define LN_OAMDMA_ADDR 0x4014

#define LN_PATTERN_0_ADDR_HEAD 0x0000
#define LN_PATTERN_ADDR_HEAD LN_PATTERN_0_ADDR_HEAD
#define LN_PATTERN_ADDR_TAIL 0x1FFF

#define LN_NT_SIZE 0x0400 // bytes
#define LN_NT_0_ADDR_HEAD 0x2000
#define LN_NT_2_ADDR_HEAD 0x2800
#define LN_NT_ADDR_HEAD LN_NT_0_ADDR_HEAD
#define LN_NT_ADDR_TAIL 0x2FFF
#define LN_NT_H_MIRROR_ADDR_MASK 0xFBFF
#define LN_NT_V_MIRROR_ADDR_MASK 0xF7FF

#define LN_NT_MIRROR_ADDR_HEAD 0x3000
#define LN_NT_MIRROR_ADDR_TAIL 0x3EFF
#define LN_NT_MIRROR_ADDR_MASK 0xEFFF

#define LN_PALETTE_ADDR_HEAD 0x3F00
#define LN_PALETTE_ADDR_TAIL 0x3FFF
#define LN_PALETTE_ADDR_MASK 0x001F          // rightmost 5 bits
#define LN_PALETTE_ADDR_BACKDROP_MASK 0x0003 // rightmost 2 bits (modulo 4)
#define LN_PALETTE_ADDR_BKG_MASK 0xFF0F      // background palette prefix
#define LN_PALETTE_ADDR_BKG_OR_MASK 0x3F00   // used to get bkg palette color

#define LN_PPU_INVALID_ADDR_HEAD 0x4000
#define LN_PPU_INVALID_ADDR_TAIL 0xFFFF
#define LN_PPU_INVALID_ADDR_MASK 0x3FFF

#define LN_SCANLINE_CYCLES 341

#define LN_APU_FC_ADDR 0x4017
