#pragma once

#define LN_NES_WIDTH 256
#define LN_NES_HEIGHT 240

#define LN_CPU_HZ 1789773

#define LN_ADDRESSABLE_SIZE (64 * 1024) // 16bit addressable memory

#define LN_INTERNAL_RAM_SIZE (2 * 1024) // 2KiB
#define LN_INTERNAL_RAM_ADDR_HEAD 0x0000
#define LN_INTERNAL_RAM_ADDR_TAIL 0x07FF

#define LN_RAM_ADDR_HEAD 0x0000
#define LN_RAM_ADDR_TAIL 0x1FFF // including mirror
#define LN_RAM_ADDR_MASK 0x07FF // leftmost 5 bits zero

#define LN_PPU_ADDRESSABLE_SIZE (16 * 1024) // 14bit addressable memory
#define LN_PPU_ADDR_MASK 0x3FFF
#define LN_PPU_INTERNAL_RAM_SIZE (2 * 1024) // 2KiB

#define LN_OAM_SP_SIZE 4
#define LN_MAX_SP_NUM 64
#define LN_OAM_SIZE (LN_MAX_SP_NUM * LN_OAM_SP_SIZE)
#define LN_MAX_VISIBLE_SP_NUM 8
#define LN_SEC_OAM_SIZE (LN_MAX_VISIBLE_SP_NUM * LN_OAM_SP_SIZE)

#define LN_PPU_REG_ADDR_HEAD 0x2000
#define LN_PPU_REG_ADDR_TAIL 0x3FFF // including mirror
#define LN_PPU_REG_ADDR_MASK 0x0007 // rightmost 3 bits 1
#define LN_OAMDMA_ADDR 0x4014

#define LN_CTRL_REG_ADDR_HEAD 0x4016
#define LN_CTRL_REG_ADDR_TAIL 0x4017

#define LN_PATTERN_TILE_WIDTH 8 // pixels
#define LN_PATTERN_TILE_HEIGHT 8
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

#define LN_PALETTE_SIZE 32
#define LN_PALETTE_ADDR_HEAD 0x3F00
#define LN_PALETTE_ADDR_TAIL 0x3FFF
#define LN_PALETTE_ADDR_MASK 0x001F          // rightmost 5 bits
#define LN_PALETTE_ADDR_BACKDROP_MASK 0x0003 // rightmost 2 bits (modulo 4)
#define LN_PALETTE_ADDR_BG_MASK 0xFF0F       // background palette prefix
// used to get background palette color
#define LN_PALETTE_ADDR_BG_OR_MASK 0x3F00
// used to get sprite palette color
#define LN_PALETTE_ADDR_SP_OR_MASK 0x3F10

#define LN_PPU_INVALID_ADDR_HEAD 0x4000
#define LN_PPU_INVALID_ADDR_TAIL 0xFFFF
#define LN_PPU_INVALID_ADDR_MASK 0x3FFF

#define LN_SCANLINE_CYCLES 341

#define LN_APU_FC_ADDR 0x4017
