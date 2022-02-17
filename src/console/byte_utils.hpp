#ifndef LN_CONSOLE_BYTEUTILS_HPP
#define LN_CONSOLE_BYTEUTILS_HPP

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
is_signed_overflow(Byte lhs, Byte rhs, Byte carry)
{
    // using extra precision.
    // int is of at least 16 bits, according to standard.
    int sum = lhs + rhs + carry;
    return sum > 127 || sum < -128;
}

} // namespace ln

#endif // LN_CONSOLE_BYTEUTILS_HPP
