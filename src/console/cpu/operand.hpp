#pragma once

#include "console/types.hpp"

namespace ln {

enum class OperandType {
    VALUE,
    ADDRESS,
    ACC, // Accumulator
};

struct Operand {
  public:
    static Operand
    from_value(Byte i_byte);
    static Operand
    from_address(Address i_addr);
    static Operand
    from_acc();

  public:
    OperandType type;
    union {
        Byte value;
        Address address;
    };

  private:
    Operand();
};

} // namespace ln
