#pragma once

#include "types.h"

inline u16
ToByte2(u8 high, u8 low)
{
    return ((u16)high << 8) + low;
}

inline void
FromByte2(u16 b2, u8 *high, u8 *low)
{
    *high = (u8)(b2 >> 8);
    *low = (u8)b2;
}

inline bool
CheckBit(unsigned int bit, u8 b)
{
    // static_assert(bit < 8, "bit must be in range [0, 7]");
    return b & (1 << bit);
}

inline bool
IsSignedOverflowAdc(u8 lhs, u8 rhs, bool carry)
{
    // using extra bits.
    i16 res = (i16)(i8)lhs + (i16)(i8)rhs + (i16)(i8)carry;
    return res > 127 || res < -128;
}

inline bool
IsSignedOverflowSbc(u8 lhs, u8 rhs, bool borrow)
{
    // using extra bits.
    i16 res = (i16)(i8)lhs - (i16)(i8)rhs - (i16)(i8)borrow;
    return res > 127 || res < -128;
}

inline void
ReverseByte(u8 *b)
{
    // Courtesy of https://stackoverflow.com/a/2602885
    *b = (*b & 0xF0) >> 4 | (*b & 0x0F) << 4;
    *b = (*b & 0xCC) >> 2 | (*b & 0x33) << 2;
    *b = (*b & 0xAA) >> 1 | (*b & 0x55) << 1;
}
