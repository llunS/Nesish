#pragma once

#include "console/cpu/cpu.hpp"

namespace nh {

struct CPU::InstrImpl {
  public:
    static void
    frm_brk(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_rti(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_rts(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_pha(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_php(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_pla(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_plp(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_jsr(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_imp(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_acc(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_imm(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_rel(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_ind_jmp(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_abs_jmp(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_abs_r(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_abs_rmw(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_abs_w(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_abx_r(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_abx_rmw(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_abx_w(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_aby_r(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_aby_rmw(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_aby_w(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_zp_r(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_zp_rmw(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_zp_w(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_zpx_r(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_zpx_rmw(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_zpx_w(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_zpy_r(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_zpy_w(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_izx_r(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_izx_rmw(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_izx_w(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);

    static void
    frm_izy_r(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_izy_rmw(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);
    static void
    frm_izy_w(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done);

  private:
    static void
    frm_phr_impl(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done,
                 Byte i_val);
    static void
    frm_plr_impl(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done,
                 Byte &o_val);
    static void
    frm_abs_pre(int i_idx, CPU *io_cpu);
    static void
    frm_abi_pre(int i_idx, CPU *io_cpu, Byte i_val);
    static void
    frm_abi_r(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done,
              Byte i_val);
    static void
    frm_abi_w(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done,
              Byte i_val);
    static void
    frm_abi_rmw(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done,
                Byte i_val);
    static void
    frm_zpi_pre(int i_idx, CPU *io_cpu, Byte i_val);
    static void
    frm_zpi_r(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done,
              Byte i_val);
    static void
    frm_zpi_w(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done,
              Byte i_val);
    static void
    frm_izx_pre(int i_idx, CPU *io_cpu);
    static void
    frm_izy_pre(int i_idx, CPU *io_cpu);

  public:
    static void
    core_nop(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_ora(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_kil(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_asl(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bpl(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_clc(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_and(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bit(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_rol(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bmi(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sec(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_eor(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_lsr(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bvc(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_cli(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_adc(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_ror(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bvs(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sei(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sta(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sty(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_stx(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_dey(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_txa(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bcc(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_tya(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_txs(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_ldy(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_lda(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_ldx(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_tay(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_tax(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bcs(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_clv(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_tsx(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_cpy(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_cmp(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_dec(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_iny(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_dex(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_bne(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_cld(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_cpx(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sbc(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_inc(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_inx(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_beq(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sed(CPU *io_cpu, Byte i_in, Byte &o_out);

    static void
    core_slo(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_anc(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_rla(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_alr(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sre(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_arr(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_rra(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_xaa(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_sax(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_las(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_lax(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_axs(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_dcp(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_isc(CPU *io_cpu, Byte i_in, Byte &o_out);

    static void
    core_tas(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_shy(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_shx(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_ahx(CPU *io_cpu, Byte i_in, Byte &o_out);

  private:
    static Byte
    core_cmp_impl(CPU *io_cpu, Byte i_in, nh::Byte i_reg);
    static void
    core_and_op(CPU *io_cpu, Byte i_in);
    static void
    core_ror_op(CPU *io_cpu, Byte i_in, Byte &o_out);
    static void
    core_XhX_impl(CPU *io_cpu, Byte i_in, Byte &o_out);

  private:
    static void
    throw_away(CPU *io_cpu, Byte i_data);
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
    test_flag_n(CPU *o_cpu, Byte i_val);
    static void
    test_flag_z(CPU *o_cpu, Byte i_val);
};

} // namespace nh
