#ifndef LN_CONSOLE_CPU_ADDRESSMODEPARSE_HPP
#define LN_CONSOLE_CPU_ADDRESSMODEPARSE_HPP

#include "console/cpu/cpu.hpp"
#include "console/types.hpp"

namespace ln {

struct CPU::AddressModeParse {
  public:
    static ln::Operand
    parse_imp(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_acc(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_imm(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_zp0(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_zpx(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_zpy(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_izx(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_izy(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_abs(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_abx(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_aby(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_ind(ln::CPU *i_cpu, bool *o_page_crossing);
    static ln::Operand
    parse_rel(ln::CPU *i_cpu, bool *o_page_crossing);

  private:
    static Byte2
    get_byte2(ln::CPU *i_cpu, Address i_address);

    static bool
    is_page_crossing(Address i_prev, Address i_current);
};

} // namespace ln

#endif // LN_CONSOLE_CPU_ADDRESSMODEPARSE_HPP
