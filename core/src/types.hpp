#pragma once

#include <cstdint>

#include "nesish/nesish.h"

namespace nh {

typedef std::uint8_t Byte;
typedef std::uint16_t Byte2;

typedef std::int8_t SignedByte;
typedef std::int16_t SignedByte2;

typedef NHCycle Cycle;
typedef NHAddr Address;

struct Uninitialized_t {};
extern Uninitialized_t Uninitialized;

} // namespace nh
