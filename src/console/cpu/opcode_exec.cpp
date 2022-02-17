#include "opcode_exec.hpp"

#include "console/byte_utils.hpp"
#include "console/assert.hpp"

#include "fmt/core.h"

#define ERROR_OPERAND_TYPE(i_cpu, i_operand)                                   \
    i_cpu->report_exec_error(                                                  \
        fmt::format("Incorrect operand type {}", i_operand.type))

namespace ln {

void
CPU::OpcodeExec::exec_nop(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_cpu);
    (void)(i_operand);
    // do nothing.
}

void
CPU::OpcodeExec::exec_brk(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->push_byte2(i_cpu->PC);
    i_cpu->push_byte(i_cpu->P);
    i_cpu->PC = 0xFFFE;

    i_cpu->set_flag(CPU::StatusFlag::B);
    i_cpu->set_flag(CPU::StatusFlag::I);
}

void
CPU::OpcodeExec::exec_php(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->push_byte(i_cpu->P);
}

void
CPU::OpcodeExec::exec_bpl(ln::CPU *i_cpu, ln::Operand i_operand)
{
    exec_branch_op(!i_cpu->check_flag(StatusFlag::N), i_cpu, i_operand);
}

void
CPU::OpcodeExec::exec_clc(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->unset_flag(StatusFlag::C);
}

void
CPU::OpcodeExec::exec_jsr(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->push_byte2(i_cpu->PC);
    ASSERT_ERROR(i_operand.type == OperandType::ADDRESS,
                 "jsr got incorrect operand type.");
    i_cpu->PC = i_operand.address;
}

void
CPU::OpcodeExec::exec_bit(ln::CPU *i_cpu, ln::Operand i_operand)
{
    ASSERT_ERROR(i_operand.type == OperandType::ADDRESS,
                 "bit got incorrect operand type.");
    Byte val = i_cpu->get_operand(i_operand);

    test_flag_n(i_cpu, val);
    i_cpu->test_flag(StatusFlag::V, check_bit<6>(val));
    test_flag_z(i_cpu, i_cpu->A & val);
}

void
CPU::OpcodeExec::exec_plp(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    exec_plp_op(i_cpu);
}

void
CPU::OpcodeExec::exec_bmi(ln::CPU *i_cpu, ln::Operand i_operand)
{
    exec_branch_op(i_cpu->check_flag(StatusFlag::N), i_cpu, i_operand);
}

void
CPU::OpcodeExec::exec_sec(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->set_flag(StatusFlag::C);
}

void
CPU::OpcodeExec::exec_rti(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    exec_plp_op(i_cpu);
    i_cpu->PC = i_cpu->pop_byte2();
}

void
CPU::OpcodeExec::exec_pha(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->push_byte(i_cpu->A);
}

void
CPU::OpcodeExec::exec_jmp(ln::CPU *i_cpu, ln::Operand i_operand)
{
    ASSERT_ERROR(i_operand.type == OperandType::ADDRESS,
                 "jmp got incorrect operand type.");
    i_cpu->PC = i_operand.address;
}

void
CPU::OpcodeExec::exec_bvc(ln::CPU *i_cpu, ln::Operand i_operand)
{
    exec_branch_op(!i_cpu->check_flag(StatusFlag::V), i_cpu, i_operand);
}

void
CPU::OpcodeExec::exec_cli(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->unset_flag(StatusFlag::I);
}

void
CPU::OpcodeExec::exec_rts(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->PC = i_cpu->pop_byte2();
}

void
CPU::OpcodeExec::exec_pla(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->A = i_cpu->pop_byte();

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
}

void
CPU::OpcodeExec::exec_bvs(ln::CPU *i_cpu, ln::Operand i_operand)
{
    exec_branch_op(i_cpu->check_flag(StatusFlag::V), i_cpu, i_operand);
}

void
CPU::OpcodeExec::exec_sei(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->set_flag(StatusFlag::I);
}

void
CPU::OpcodeExec::exec_sty(ln::CPU *i_cpu, ln::Operand i_operand)
{
    auto err = i_cpu->set_operand(i_operand, i_cpu->Y);
    if (LN_FAILED(err))
    {
        ERROR_OPERAND_TYPE(i_cpu, i_operand);
    }
}

void
CPU::OpcodeExec::exec_dey(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    --i_cpu->Y;

    test_flag_n(i_cpu, i_cpu->Y);
    test_flag_z(i_cpu, i_cpu->Y);
}

void
CPU::OpcodeExec::exec_bcc(ln::CPU *i_cpu, ln::Operand i_operand)
{
    exec_branch_op(!i_cpu->check_flag(StatusFlag::C), i_cpu, i_operand);
}

void
CPU::OpcodeExec::exec_tya(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->A = i_cpu->Y;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
}

void
CPU::OpcodeExec::exec_shy(ln::CPU *i_cpu, ln::Operand i_operand)
{
    // @UNCERTAIN
    // https://github.com/ltriant/nes/blob/master/doc/undocumented_opcodes.txt
    ASSERT_ERROR(i_operand.type == OperandType::ADDRESS,
                 "shy got incorrect operand type.");
    Byte val = i_cpu->Y & ((i_operand.address >> 8) + 1);
    i_cpu->set_byte(i_operand.address, val);
}

void
CPU::OpcodeExec::exec_ldy(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);
    i_cpu->Y = val;

    test_flag_n(i_cpu, i_cpu->Y);
    test_flag_z(i_cpu, i_cpu->Y);
}

void
CPU::OpcodeExec::exec_tay(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->Y = i_cpu->A;

    test_flag_n(i_cpu, i_cpu->Y);
    test_flag_z(i_cpu, i_cpu->Y);
}

void
CPU::OpcodeExec::exec_bcs(ln::CPU *i_cpu, ln::Operand i_operand)
{
    exec_branch_op(i_cpu->check_flag(StatusFlag::C), i_cpu, i_operand);
}

void
CPU::OpcodeExec::exec_clv(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->unset_flag(StatusFlag::V);
}

void
CPU::OpcodeExec::exec_cpy(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);
    exec_cmp_full(i_cpu, val, i_cpu->Y);
}

void
CPU::OpcodeExec::exec_iny(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    ++i_cpu->Y;

    test_flag_n(i_cpu, i_cpu->Y);
    test_flag_z(i_cpu, i_cpu->Y);
}

void
CPU::OpcodeExec::exec_bne(ln::CPU *i_cpu, ln::Operand i_operand)
{
    exec_branch_op(!i_cpu->check_flag(StatusFlag::Z), i_cpu, i_operand);
}

void
CPU::OpcodeExec::exec_cld(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->unset_flag(StatusFlag::D);
}

void
CPU::OpcodeExec::exec_cpx(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);
    exec_cmp_full(i_cpu, val, i_cpu->X);
}

void
CPU::OpcodeExec::exec_inx(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    ++i_cpu->X;

    test_flag_n(i_cpu, i_cpu->X);
    test_flag_z(i_cpu, i_cpu->X);
}

void
CPU::OpcodeExec::exec_beq(ln::CPU *i_cpu, ln::Operand i_operand)
{
    exec_branch_op(i_cpu->check_flag(StatusFlag::Z), i_cpu, i_operand);
}

void
CPU::OpcodeExec::exec_sed(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->set_flag(StatusFlag::D);
}

void
CPU::OpcodeExec::exec_ora(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);
    i_cpu->A |= val;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
}

void
CPU::OpcodeExec::exec_and(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);
    i_cpu->A &= val;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
}

void
CPU::OpcodeExec::exec_eor(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);
    i_cpu->A ^= val;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
}

void
CPU::OpcodeExec::exec_adc(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);
    exec_adc_full(i_cpu, val);
}

void
CPU::OpcodeExec::exec_sta(ln::CPU *i_cpu, ln::Operand i_operand)
{
    auto err = i_cpu->set_operand(i_operand, i_cpu->A);
    if (LN_FAILED(err))
    {
        ERROR_OPERAND_TYPE(i_cpu, i_operand);
    }
}

void
CPU::OpcodeExec::exec_lda(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);
    i_cpu->A = val;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
}

void
CPU::OpcodeExec::exec_cmp(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);
    exec_cmp_full(i_cpu, val, i_cpu->A);
}

void
CPU::OpcodeExec::exec_sbc(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);
    exec_sbc_full(i_cpu, val);
}

void
CPU::OpcodeExec::exec_kil(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->halt();
}

void
CPU::OpcodeExec::exec_asl(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = exec_asl_op(i_cpu, i_operand, val);

    test_flag_n(i_cpu, new_val);
    test_flag_z(i_cpu, new_val);
    i_cpu->test_flag(StatusFlag::C, check_bit<7>(val));
}

void
CPU::OpcodeExec::exec_rol(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = exec_rol_op(i_cpu, i_operand, val);

    test_flag_n(i_cpu, new_val);
    test_flag_z(i_cpu, new_val);
    i_cpu->test_flag(StatusFlag::C, check_bit<7>(val));
}

void
CPU::OpcodeExec::exec_lsr(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = exec_lsr_op(i_cpu, i_operand, val);

    test_flag_n(i_cpu, new_val);
    test_flag_z(i_cpu, new_val);
    i_cpu->test_flag(StatusFlag::C, check_bit<0>(val));
}

void
CPU::OpcodeExec::exec_ror(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = exec_ror_op(i_cpu, i_operand, val);

    test_flag_n(i_cpu, new_val);
    test_flag_z(i_cpu, new_val);
    i_cpu->test_flag(StatusFlag::C, check_bit<0>(val));
}

void
CPU::OpcodeExec::exec_stx(ln::CPU *i_cpu, ln::Operand i_operand)
{
    auto err = i_cpu->set_operand(i_operand, i_cpu->X);
    if (LN_FAILED(err))
    {
        ERROR_OPERAND_TYPE(i_cpu, i_operand);
    }
}

void
CPU::OpcodeExec::exec_txa(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->A = i_cpu->X;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
}

void
CPU::OpcodeExec::exec_txs(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->S = i_cpu->X;
}

void
CPU::OpcodeExec::exec_shx(ln::CPU *i_cpu, ln::Operand i_operand)
{
    // @UNCERTAIN
    // https://github.com/ltriant/nes/blob/master/doc/undocumented_opcodes.txt
    ASSERT_ERROR(i_operand.type == OperandType::ADDRESS,
                 "shy got incorrect operand type.");
    Byte val = i_cpu->X & ((i_operand.address >> 8) + 1);
    i_cpu->set_byte(i_operand.address, val);
}

void
CPU::OpcodeExec::exec_ldx(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    i_cpu->X = val;

    test_flag_n(i_cpu, i_cpu->X);
    test_flag_z(i_cpu, i_cpu->X);
}

void
CPU::OpcodeExec::exec_tax(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->X = i_cpu->A;
    (void)(i_cpu);

    test_flag_n(i_cpu, i_cpu->X);
    test_flag_z(i_cpu, i_cpu->X);
}

void
CPU::OpcodeExec::exec_tsx(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    i_cpu->X = i_cpu->S;
    (void)(i_cpu);

    test_flag_n(i_cpu, i_cpu->X);
    test_flag_z(i_cpu, i_cpu->X);
}

void
CPU::OpcodeExec::exec_dec(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = exec_dec_op(i_cpu, i_operand, val);

    test_flag_n(i_cpu, new_val);
    test_flag_z(i_cpu, new_val);
}

void
CPU::OpcodeExec::exec_dex(ln::CPU *i_cpu, ln::Operand i_operand)
{
    (void)(i_operand);

    --i_cpu->X;

    test_flag_n(i_cpu, i_cpu->X);
    test_flag_z(i_cpu, i_cpu->X);
}

void
CPU::OpcodeExec::exec_inc(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = exec_inc_op(i_cpu, i_operand, val);

    test_flag_n(i_cpu, new_val);
    test_flag_z(i_cpu, new_val);
}

void
CPU::OpcodeExec::exec_slo(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = exec_asl_op(i_cpu, i_operand, val);
    i_cpu->A |= new_val;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
    i_cpu->test_flag(StatusFlag::C, check_bit<7>(val));
}

void
CPU::OpcodeExec::exec_anc(ln::CPU *i_cpu, ln::Operand i_operand)
{
    // https://www.nesdev.com/extra_instructions.txt
    // http://www.oxyron.de/html/opcodes02.html
    Byte val = i_cpu->get_operand(i_operand);

    i_cpu->A &= val;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
    i_cpu->test_flag(StatusFlag::C, check_bit<7>(i_cpu->A));
}

void
CPU::OpcodeExec::exec_rla(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = exec_rol_op(i_cpu, i_operand, val);
    i_cpu->A &= new_val;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
    i_cpu->test_flag(StatusFlag::C, check_bit<7>(val));
}

void
CPU::OpcodeExec::exec_sre(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = exec_lsr_op(i_cpu, i_operand, val);
    i_cpu->A ^= new_val;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
    i_cpu->test_flag(StatusFlag::C, check_bit<0>(val));
}

void
CPU::OpcodeExec::exec_alr(ln::CPU *i_cpu, ln::Operand i_operand)
{
    // http://www.oxyron.de/html/opcodes02.html
    Byte val = i_cpu->get_operand(i_operand);

    Byte and_val = i_cpu->A & val;
    // LSR A
    exec_lsr_op(i_cpu, Operand::from_acc(), and_val);

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
    i_cpu->test_flag(StatusFlag::C, check_bit<0>(and_val));
}

void
CPU::OpcodeExec::exec_rra(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte ror_value = exec_ror_op(i_cpu, i_operand, val);
    exec_adc_full(i_cpu, ror_value);
}

void
CPU::OpcodeExec::exec_arr(ln::CPU *i_cpu, ln::Operand i_operand)
{
    // http://www.oxyron.de/html/opcodes02.html
    Byte val = i_cpu->get_operand(i_operand);

    Byte and_val = i_cpu->A & val;
    // ROR A
    exec_ror_op(i_cpu, Operand::from_acc(), and_val);

    test_flag_n(i_cpu, i_cpu->A);
    i_cpu->test_flag(StatusFlag::V, is_signed_overflow(and_val, val, 0));
    test_flag_z(i_cpu, i_cpu->A);
    i_cpu->test_flag(StatusFlag::C, check_bit<7>(and_val));
}

void
CPU::OpcodeExec::exec_sax(ln::CPU *i_cpu, ln::Operand i_operand)
{
    i_cpu->set_operand(i_operand, i_cpu->A & i_cpu->X);
}

void
CPU::OpcodeExec::exec_xaa(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    i_cpu->A = i_cpu->X & val;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
}

void
CPU::OpcodeExec::exec_ahx(ln::CPU *i_cpu, ln::Operand i_operand)
{
    // @UNCERTAIN
    // http://www.oxyron.de/html/opcodes02.html
    // https://www.nesdev.com/extra_instructions.txt
    ASSERT_ERROR(i_operand.type == OperandType::ADDRESS,
                 "shy got incorrect operand type.");
    Byte val = i_cpu->A & i_cpu->X & ((i_operand.address >> 8) + 1);
    i_cpu->set_byte(i_operand.address, val);
}

void
CPU::OpcodeExec::exec_tas(ln::CPU *i_cpu, ln::Operand i_operand)
{
    // @UNCERTAIN
    // https://www.nesdev.com/extra_instructions.txt
    // https://www.nesdev.com/extra_instructions.txt
    i_cpu->S = i_cpu->A & i_cpu->X;

    Byte val = i_cpu->S & ((i_operand.address >> 8) + 1);
    i_cpu->set_byte(i_operand.address, val);
}

void
CPU::OpcodeExec::exec_lax(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    i_cpu->A = val;
    i_cpu->X = val;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
}

void
CPU::OpcodeExec::exec_las(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = val & i_cpu->S;

    i_cpu->A = i_cpu->X = i_cpu->S = new_val;

    test_flag_n(i_cpu, i_cpu->A);
    test_flag_z(i_cpu, i_cpu->A);
}

void
CPU::OpcodeExec::exec_dcp(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = exec_dec_op(i_cpu, i_operand, val);
    exec_cmp_full(i_cpu, new_val, i_cpu->A);
}

void
CPU::OpcodeExec::exec_axs(ln::CPU *i_cpu, ln::Operand i_operand)
{
    // @UNCERTAIN
    // http://www.oxyron.de/html/opcodes02.html
    Byte val = i_cpu->get_operand(i_operand);

    Byte a_and_x = i_cpu->A & i_cpu->X;
    Byte delta = exec_cmp_full(i_cpu, val, a_and_x);
    i_cpu->X = delta;
}

void
CPU::OpcodeExec::exec_isc(ln::CPU *i_cpu, ln::Operand i_operand)
{
    Byte val = i_cpu->get_operand(i_operand);

    Byte new_val = exec_inc_op(i_cpu, i_operand, val);
    exec_sbc_full(i_cpu, new_val);
}

void
CPU::OpcodeExec::exec_branch_op(bool i_cond, ln::CPU *i_cpu,
                                ln::Operand i_operand)
{
    int offset = (int)i_cpu->get_operand(i_operand);

    if (i_cond)
    {
        i_cpu->PC += offset;

        // detect infinite loop.
        if (offset == -2)
        {
            ln::get_logger()->critical("Infinite loop detected.");
        }
    }
}

void
CPU::OpcodeExec::exec_plp_op(ln::CPU *i_cpu)
{
    Byte preserve_mask = (StatusFlag::B | StatusFlag::U);
    // ignore the mask for stack value and preserve the mask in original P.
    i_cpu->P =
        (i_cpu->pop_byte() & ~preserve_mask) | (i_cpu->P & preserve_mask);
}

void
CPU::OpcodeExec::exec_adc_full(ln::CPU *i_cpu, Byte i_val)
{
    bool carry = i_cpu->check_flag(StatusFlag::C);

    Byte prev_val = i_cpu->A;
    i_cpu->A += i_val + carry;

    // either new value < previous value, or the are the same even if carry
    // exists.
    bool unsigned_overflow =
        (i_cpu->A < prev_val || (i_cpu->A == prev_val && carry));
    bool signed_overflow = is_signed_overflow(prev_val, i_val, carry);

    test_flag_n(i_cpu, i_cpu->A);
    i_cpu->test_flag(StatusFlag::V, signed_overflow);
    test_flag_z(i_cpu, i_cpu->A);
    i_cpu->test_flag(StatusFlag::C, unsigned_overflow);
}

void
CPU::OpcodeExec::exec_sbc_full(ln::CPU *i_cpu, Byte i_val)
{
    bool borrow = !i_cpu->check_flag(StatusFlag::C);

    Byte prev_val = i_cpu->A;
    i_cpu->A += -i_val - borrow;

    bool no_borrow = prev_val >= i_val;
    bool signed_overflow = is_signed_overflow(prev_val, -i_val, -borrow);

    test_flag_n(i_cpu, i_cpu->A);
    i_cpu->test_flag(StatusFlag::V, signed_overflow);
    test_flag_z(i_cpu, i_cpu->A);
    i_cpu->test_flag(StatusFlag::C, no_borrow);
}

Byte
CPU::OpcodeExec::exec_cmp_full(ln::CPU *i_cpu, Byte i_byte, ln::Byte i_register)
{
    Byte delta = i_register - i_byte;
    bool borrow = i_register < i_byte;

    test_flag_n(i_cpu, delta);
    test_flag_z(i_cpu, delta);
    i_cpu->test_flag(StatusFlag::C, !borrow);

    return delta;
}

Byte
CPU::OpcodeExec::exec_asl_op(ln::CPU *i_cpu, ln::Operand i_write_operand,
                             Byte i_val)
{
    Byte new_val = i_val << 1;
    auto err = i_cpu->set_operand(i_write_operand, new_val);
    if (LN_FAILED(err))
    {
        ERROR_OPERAND_TYPE(i_cpu, i_write_operand);
    }

    return new_val;
}

Byte
CPU::OpcodeExec::exec_rol_op(ln::CPU *i_cpu, ln::Operand i_write_operand,
                             Byte i_val)
{
    bool carry = i_cpu->check_flag(StatusFlag::C);

    Byte new_val = (i_val << 1) | (carry << 0);
    auto err = i_cpu->set_operand(i_write_operand, new_val);
    if (LN_FAILED(err))
    {
        ERROR_OPERAND_TYPE(i_cpu, i_write_operand);
    }

    return new_val;
}

Byte
CPU::OpcodeExec::exec_lsr_op(ln::CPU *i_cpu, ln::Operand i_write_operand,
                             Byte i_val)
{
    Byte new_val = i_val >> 1;
    auto err = i_cpu->set_operand(i_write_operand, new_val);
    if (LN_FAILED(err))
    {
        ERROR_OPERAND_TYPE(i_cpu, i_write_operand);
    }

    return new_val;
}

Byte
CPU::OpcodeExec::exec_ror_op(ln::CPU *i_cpu, ln::Operand i_write_operand,
                             Byte i_val)
{
    bool carry = i_cpu->check_flag(StatusFlag::C);

    Byte new_val = (i_val >> 1) | (carry << 7);
    auto err = i_cpu->set_operand(i_write_operand, new_val);
    if (LN_FAILED(err))
    {
        ERROR_OPERAND_TYPE(i_cpu, i_write_operand);
    }

    return new_val;
}

Byte
CPU::OpcodeExec::exec_dec_op(ln::CPU *i_cpu, ln::Operand i_write_operand,
                             Byte i_val)
{
    Byte new_val = i_val - 1;
    auto err = i_cpu->set_operand(i_write_operand, new_val);
    if (LN_FAILED(err))
    {
        ERROR_OPERAND_TYPE(i_cpu, i_write_operand);
    }

    return new_val;
}

Byte
CPU::OpcodeExec::exec_inc_op(ln::CPU *i_cpu, ln::Operand i_write_operand,
                             Byte i_val)
{
    Byte new_val = i_val + 1;
    auto err = i_cpu->set_operand(i_write_operand, new_val);
    if (LN_FAILED(err))
    {
        ERROR_OPERAND_TYPE(i_cpu, i_write_operand);
    }

    return new_val;
}

void
CPU::OpcodeExec::test_flag_n(ln::CPU *i_cpu, Byte i_val)
{
    i_cpu->test_flag(StatusFlag::N, check_bit<7>(i_val));
}

void
CPU::OpcodeExec::test_flag_z(ln::CPU *i_cpu, Byte i_val)
{
    i_cpu->test_flag(StatusFlag::Z, i_val == 0);
}

} // namespace ln
