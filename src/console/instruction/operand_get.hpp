#ifndef LN_CONSOLE_INSTRUCTION_OPERANDGET_HPP
#define LN_CONSOLE_INSTRUCTION_OPERANDGET_HPP

#include "console/cpu.hpp"
#include "console/types.hpp"

namespace ln {

struct CPU::OperandGet {
  public:
    typedef Byte (*GetFunc)(ln::CPU *i_cpu, ln::Memory *i_memory);

    static Byte
    get_imp(ln::CPU *i_cpu, ln::Memory *i_memory);
    static Byte
    get_imm(ln::CPU *i_cpu, ln::Memory *i_memory);
    static Byte
    get_zp0(ln::CPU *i_cpu, ln::Memory *i_memory);
    static Byte
    get_zpx(ln::CPU *i_cpu, ln::Memory *i_memory);
    static Byte
    get_zpy(ln::CPU *i_cpu, ln::Memory *i_memory);
    static Byte
    get_izx(ln::CPU *i_cpu, ln::Memory *i_memory);
    static Byte
    get_izy(ln::CPU *i_cpu, ln::Memory *i_memory);
    static Byte
    get_abs(ln::CPU *i_cpu, ln::Memory *i_memory);
    static Byte
    get_abx(ln::CPU *i_cpu, ln::Memory *i_memory);
    static Byte
    get_aby(ln::CPU *i_cpu, ln::Memory *i_memory);
    static Byte
    get_ind(ln::CPU *i_cpu, ln::Memory *i_memory);
    static Byte
    get_rel(ln::CPU *i_cpu, ln::Memory *i_memory);
};

} // namespace ln

#endif // LN_CONSOLE_INSTRUCTION_OPERANDGET_HPP
