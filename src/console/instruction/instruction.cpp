#include "instruction.hpp"

#include "console/instruction/instruction_map.hpp"

namespace ln {

Instruction::Instruction(OpCode i_op_code, AddressMode i_address_mode)
    : op_code(i_op_code)
    , address_mode(i_address_mode)
{
}

Instruction
Instruction::decode(Byte i_raw_instruction)
{
    return g_instruction_map[i_raw_instruction];
}

} // namespace ln
