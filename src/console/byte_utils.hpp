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
    // int is of at least 16 bits, according to standard.
    int result = (SignedByte)i_lhs + (SignedByte)i_rhs + (SignedByte)i_carry;
    return result > 127 || result < -128;
}

inline bool
is_signed_overflow_sbc(Byte i_lhs, Byte i_rhs, bool i_borrow)
{
    // using extra bits.
    // int is of at least 16 bits, according to standard.
    int result = (SignedByte)i_lhs - (SignedByte)i_rhs - (SignedByte)i_borrow;
    return result > 127 || result < -128;
}

} // namespace ln
