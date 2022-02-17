#include "operand.hpp"

namespace ln {

Operand::Operand()
{
}

Operand
Operand::from_value(Byte i_byte)
{
    Operand inst;
    inst.type = OperandType::VALUE;
    inst.value = i_byte;
    return inst;
}

Operand
Operand::from_address(Address i_address)
{
    Operand inst;
    inst.type = OperandType::ADDRESS;
    inst.address = i_address;
    return inst;
}

Operand
Operand::from_acc()
{
    Operand inst;
    inst.type = OperandType::ACC;
    return inst;
}

} // namespace ln
