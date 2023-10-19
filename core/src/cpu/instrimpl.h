#pragma once

#include "cpu/instrdesc.h"

void
cpu_frm_brk(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_rti(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_rts(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_pha(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_php(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_pla(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_plp(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_jsr(int idx, cpu_s *cpu, instrcore_f core, bool *done);

void
cpu_frm_imp(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_acc(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_imm(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_rel(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_ind_jmp(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_abs_jmp(int idx, cpu_s *cpu, instrcore_f core, bool *done);

void
cpu_frm_abs_r(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_abs_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_abs_w(int idx, cpu_s *cpu, instrcore_f core, bool *done);

void
cpu_frm_abx_r(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_abx_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_abx_w(int idx, cpu_s *cpu, instrcore_f core, bool *done);

void
cpu_frm_aby_r(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_aby_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_aby_w(int idx, cpu_s *cpu, instrcore_f core, bool *done);

void
cpu_frm_zp_r(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_zp_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_zp_w(int idx, cpu_s *cpu, instrcore_f core, bool *done);

void
cpu_frm_zpx_r(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_zpx_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_zpx_w(int idx, cpu_s *cpu, instrcore_f core, bool *done);

void
cpu_frm_zpy_r(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_zpy_w(int idx, cpu_s *cpu, instrcore_f core, bool *done);

void
cpu_frm_izx_r(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_izx_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_izx_w(int idx, cpu_s *cpu, instrcore_f core, bool *done);

void
cpu_frm_izy_r(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_izy_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done);
void
cpu_frm_izy_w(int idx, cpu_s *cpu, instrcore_f core, bool *done);

void
cpu_core_nop(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_ora(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_kil(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_asl(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_bpl(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_clc(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_and(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_bit(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_rol(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_bmi(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_sec(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_eor(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_lsr(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_bvc(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_cli(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_adc(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_ror(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_bvs(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_sei(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_sta(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_sty(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_stx(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_dey(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_txa(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_bcc(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_tya(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_txs(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_ldy(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_lda(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_ldx(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_tay(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_tax(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_bcs(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_clv(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_tsx(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_cpy(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_cmp(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_dec(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_iny(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_dex(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_bne(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_cld(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_cpx(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_sbc(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_inc(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_inx(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_beq(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_sed(cpu_s *cpu, u8 in, u8 *out);

void
cpu_core_slo(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_anc(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_rla(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_alr(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_sre(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_arr(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_rra(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_xaa(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_sax(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_las(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_lax(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_axs(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_dcp(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_isc(cpu_s *cpu, u8 in, u8 *out);

void
cpu_core_tas(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_shy(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_shx(cpu_s *cpu, u8 in, u8 *out);
void
cpu_core_ahx(cpu_s *cpu, u8 in, u8 *out);
