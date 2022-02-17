#ifndef LN_CONSOLE_CPU_OPERAND_HPP
#define LN_CONSOLE_CPU_OPERAND_HPP

#include "console/types.hpp"

namespace ln {

enum class OperandType
{
    VALUE,
    ADDRESS,
    ACC, // Accumulator
};

struct Operand {
  public:
    static Operand
    from_value(Byte i_byte);
    static Operand
    from_address(Address i_address);
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

#endif // LN_CONSOLE_CPU_OPERAND_HPP
