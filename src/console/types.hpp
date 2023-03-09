#pragma once

#include <cstdint>
#include <cstddef>

namespace nh {

typedef std::uint8_t Byte;
typedef std::uint16_t Byte2;

typedef std::int8_t SignedByte;
typedef std::int16_t SignedByte2;

typedef std::size_t Cycle;
typedef Byte2 Address;

struct Uninitialized_t {};
extern Uninitialized_t Uninitialized;

} // namespace nh
