#ifndef LN_CONSOLE_OPERANDGET_HPP
#define LN_CONSOLE_OPERANDGET_HPP

#include "console/cpu.hpp"

namespace ln {

struct CPU::OperandGet {
  public:
    typedef void (*GetFunc)(ln::CPU *i_cpu, ln::Memory *i_memory,
                            uint8_t *o_operand);

    static void
    get_imp(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
    static void
    get_imm(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
    static void
    get_zp0(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
    static void
    get_zpx(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
    static void
    get_zpy(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
    static void
    get_izx(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
    static void
    get_izy(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
    static void
    get_abs(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
    static void
    get_abx(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
    static void
    get_aby(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
    static void
    get_ind(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
    static void
    get_rel(ln::CPU *i_cpu, ln::Memory *i_memory, uint8_t *o_operand);
};

} // namespace ln

#endif // LN_CONSOLE_OPERANDGET_HPP
