#pragma once

#include "console/cpu/cpu.hpp"

namespace ln {

struct CPU::InstrImpl {
  public:
    static void
    frm_brk(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_rti(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_rts(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_pha(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_php(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_pla(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_plp(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_jsr(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_imp(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_acc(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_imm(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_rel(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_ind_jmp(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_abs_jmp(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_abs_r(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_abs_rmw(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_abs_w(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_abx_r(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_abx_rmw(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_abx_w(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_aby_r(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_aby_rmw(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_aby_w(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_zp_r(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_zp_rmw(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_zp_w(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_zpx_r(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_zpx_rmw(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_zpx_w(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_zpy_r(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_zpy_w(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_izx_r(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_izx_rmw(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_izx_w(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_izy_r(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_izy_rmw(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_izy_w(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done);

  private:
    static void
    frm_phr_impl(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done,
                 Byte i_val);
    static void
    frm_plr_impl(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done,
                 Byte &o_val);
    static void
    frm_abs_pre(int i_idx, ln::CPU *io_cpu);
    static void
    frm_abi_pre(int i_idx, ln::CPU *io_cpu, Byte i_val);
    static void
    frm_abi_r(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done,
              Byte i_val);
    static void
    frm_abi_w(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done,
              Byte i_val);
    static void
    frm_abi_rmw(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done,
                Byte i_val);
    static void
    frm_zpi_pre(int i_idx, ln::CPU *io_cpu, Byte i_val);
    static void
    frm_zpi_r(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done,
              Byte i_val);
    static void
    frm_zpi_w(int i_idx, ln::CPU *io_cpu, InstrCore i_core, bool &io_done,
              Byte i_val);
    static void
    frm_izx_pre(int i_idx, ln::CPU *io_cpu);
    static void
    frm_izy_pre(int i_idx, ln::CPU *io_cpu);

  public:
    static void
    core_nop(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_ora(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_kil(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_asl(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bpl(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_clc(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_and(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bit(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_rol(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bmi(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sec(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_eor(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_lsr(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bvc(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_cli(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_adc(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_ror(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bvs(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sei(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sta(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sty(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_stx(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_dey(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_txa(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bcc(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_tya(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_txs(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_ldy(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_lda(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_ldx(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_tay(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_tax(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bcs(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_clv(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_tsx(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_cpy(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_cmp(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_dec(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_iny(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_dex(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bne(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_cld(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_cpx(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sbc(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_inc(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_inx(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_beq(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sed(ln::CPU *io_cpu, Byte i_in, Byte &o_out);

    static void
    core_slo(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_anc(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_rla(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_alr(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sre(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_arr(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_rra(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_xaa(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sax(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_las(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_lax(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_axs(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_dcp(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_isc(ln::CPU *io_cpu, Byte i_in, Byte &o_out);

    static void
    core_tas(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_shy(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_shx(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_ahx(ln::CPU *io_cpu, Byte i_in, Byte &o_out);

  private:
    static Byte
    core_cmp_impl(ln::CPU *io_cpu, Byte i_in, ln::Byte i_reg);
    static void
    core_and_op(ln::CPU *io_cpu, Byte i_in);
    static void
    core_ror_op(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_XhX_impl(ln::CPU *io_cpu, Byte i_in, Byte &o_out);

  private:
    static void
    throw_away(ln::CPU *io_cpu, Byte i_data);
    static void
    ignore_ub(Byte i_src, Byte &io_dst);
    static void
    set_low(Address &o_dst, Byte i_val);
    static void
    set_high(Address &o_dst, Byte i_val);
    static Byte
    get_low(Address i_val);
    static Byte
    get_high(Address i_val);

    static void
    test_flag_n(ln::CPU *o_cpu, Byte i_val);
    static void
    test_flag_z(ln::CPU *o_cpu, Byte i_val);
};

} // namespace ln
