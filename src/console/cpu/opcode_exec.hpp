#ifndef LN_CONSOLE_CPU_OPCODEEXEC_HPP
#define LN_CONSOLE_CPU_OPCODEEXEC_HPP

#include "console/cpu/cpu.hpp"

namespace ln {

struct CPU::OpcodeExec {
  public:
    static void
    exec_nop(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);

    static void
    exec_brk(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_php(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_bpl(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_clc(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_jsr(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_bit(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_plp(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_bmi(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_sec(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_rti(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_pha(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_jmp(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_bvc(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_cli(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_rts(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_pla(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_bvs(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_sei(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_sty(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_dey(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_bcc(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_tya(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_shy(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_ldy(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_tay(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_bcs(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_clv(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_cpy(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_iny(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_bne(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_cld(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_cpx(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_inx(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_beq(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_sed(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);

    static void
    exec_ora(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_and(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_eor(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_adc(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_sta(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_lda(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_cmp(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_sbc(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);

    static void
    exec_kil(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_asl(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_rol(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_lsr(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_ror(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_stx(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_txa(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_txs(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_shx(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_ldx(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_tax(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_tsx(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_dec(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_dex(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_inc(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);

    static void
    exec_slo(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_anc(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_rla(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_sre(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_alr(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_rra(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_arr(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_sax(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_xaa(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_ahx(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_tas(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_lax(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_las(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_dcp(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_axs(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);
    static void
    exec_isc(ln::CPU *i_cpu, ln::Operand i_operand, Cycle &o_branch_cycles);

  private:
    static void
    test_flag_n(ln::CPU *i_cpu, Byte i_val);
    static void
    test_flag_z(ln::CPU *i_cpu, Byte i_val);

    static void
    exec_branch_op(bool i_cond, ln::CPU *i_cpu, ln::Operand i_operand,
                   Cycle &o_branch_cycles);

    static Byte
    exec_and_op(ln::CPU *i_cpu, Byte i_val);
    static Byte
    exec_ror_op(ln::CPU *i_cpu, ln::Operand i_write_operand, Byte i_val);

    static void
    exec_plp_full(ln::CPU *i_cpu);

    static Byte
    exec_asl_full(ln::CPU *i_cpu, ln::Operand i_write_operand, Byte i_val);
    static Byte
    exec_rol_full(ln::CPU *i_cpu, ln::Operand i_write_operand, Byte i_val);
    static Byte
    exec_lsr_full(ln::CPU *i_cpu, ln::Operand i_write_operand, Byte i_val);
    static Byte
    exec_ror_full(ln::CPU *i_cpu, ln::Operand i_write_operand, Byte i_val);

    static Byte
    exec_dec_full(ln::CPU *i_cpu, ln::Operand i_write_operand, Byte i_val);
    static Byte
    exec_inc_full(ln::CPU *i_cpu, ln::Operand i_write_operand, Byte i_val);

    static void
    exec_ora_full(ln::CPU *i_cpu, Byte i_val);
    static Byte
    exec_and_full(ln::CPU *i_cpu, Byte i_val);
    static void
    exec_eor_full(ln::CPU *i_cpu, Byte i_val);

    static Byte
    exec_cmp_full(ln::CPU *i_cpu, Byte i_byte, ln::Byte i_register);
    static void
    exec_adc_full(ln::CPU *i_cpu, Byte i_val);
    static void
    exec_sbc_full(ln::CPU *i_cpu, Byte i_val);
};

} // namespace ln

#endif // LN_CONSOLE_CPU_OPCODEEXEC_HPP