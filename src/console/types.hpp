#pragma once

#include <cstdint>

namespace ln {

typedef std::uint8_t Byte;
typedef std::uint16_t Byte2;
typedef Byte2 Address;

typedef std::int8_t SignedByte;

struct Uninitialized_t {};
extern Uninitialized_t Uninitialized;

} // namespace ln
