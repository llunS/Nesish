#ifndef LN_CONSOLE_CPU_ADDRESSMODEPARSE_HPP
#define LN_CONSOLE_CPU_ADDRESSMODEPARSE_HPP

#include "console/cpu/cpu.hpp"
#include "console/types.hpp"

namespace ln {

struct CPU::AddressModeParse {
  public:
    static ln::Operand
    parse_imp(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_acc(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_imm(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_zp0(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_zpx(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_zpy(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_izx(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_izy(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_abs(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_abx(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_aby(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_ind(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);
    static ln::Operand
    parse_rel(const ln::CPU *i_cpu, Byte &o_operand_bytes,
              bool *o_page_crossing);

  private:
    static Byte2
    get_byte2(const ln::CPU *i_cpu, Address i_addr);

    static bool
    is_page_crossing(Address i_prev, Address i_current);
};

} // namespace ln

#endif // LN_CONSOLE_CPU_ADDRESSMODEPARSE_HPP
