#ifndef LN_CONSOLE_SPEC_HPP
#define LN_CONSOLE_SPEC_HPP

// Resolution
#define LN_NES_WIDTH 256
#define LN_NES_HEIGHT 240

#define LN_ADDRESSABLE_SIZE (64 * 1024) // 16bit addressable memory

#define LN_INTERNAL_RAM_SIZE (2 * 1024) // 2KiB
#define LN_INTERNAL_RAM_ADDRESS_HEAD 0x0000
#define LN_INTERNAL_RAM_ADDRESS_TAIL 0x07FF
#define LN_INTERNAL_RAM_MASK 0x07FF // leftmost 5 bits zero

#define LN_RAM_ADDRESS_HEAD 0x0000
#define LN_RAM_ADDRESS_TAIL 0x1FFF
#define LN_RAM_MASK 0x1FFF // leftmost 3 bits zero

#define LN_APU_FC_ADDRESS 0x4017

#endif // LN_CONSOLE_SPEC_HPP
