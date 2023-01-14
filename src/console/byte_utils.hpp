#pragma once

#include "console/types.hpp"

namespace ln {

inline Byte2
byte2_from_bytes(Byte i_higher, Byte i_lower)
{
    return ((Byte2)i_higher << 8) + i_lower;
}

template <unsigned int bit>
bool
check_bit(Byte i_byte)
{
    static_assert(bit < 8, "bit must be in range [0, 7].");
    return i_byte & (1 << bit);
}

inline bool
is_signed_overflow_adc(Byte i_lhs, Byte i_rhs, bool i_carry)
{
    // using extra bits.
    SignedByte2 result = (SignedByte2)(SignedByte)i_lhs +
                         (SignedByte2)(SignedByte)i_rhs +
                         (SignedByte2)(SignedByte)i_carry;
    return result > 127 || result < -128;
}

inline bool
is_signed_overflow_sbc(Byte i_lhs, Byte i_rhs, bool i_borrow)
{
    // using extra bits.
    SignedByte2 result = (SignedByte2)(SignedByte)i_lhs -
                         (SignedByte2)(SignedByte)i_rhs -
                         (SignedByte2)(SignedByte)i_borrow;
    return result > 127 || result < -128;
}

inline void
byte_reverse(Byte &io_byte)
{
    // Courtesy of https://stackoverflow.com/a/2602885
    io_byte = (io_byte & 0xF0) >> 4 | (io_byte & 0x0F) << 4;
    io_byte = (io_byte & 0xCC) >> 2 | (io_byte & 0x33) << 2;
    io_byte = (io_byte & 0xAA) >> 1 | (io_byte & 0x55) << 1;
}

} // namespace ln
