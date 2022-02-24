#include "console/cpu/cpu.hpp"
#include "console/cpu/opcode_exec.hpp"

#define EXEC_OF(i_opcode_name) CPU::OpcodeExec::exec_##i_opcode_name

namespace ln {

// Maybe we should write a script to generate this...

CPU::InstructionDesc CPU::s_instr_map[256] = {
    // http://www.oxyron.de/html/opcodes02.html
    /* clang-format off */
    // -- 0x
    {OpcodeType::BRK, AddressMode::IMP, EXEC_OF(brk), 7, 0},
    {OpcodeType::ORA, AddressMode::IZX, EXEC_OF(ora), 6, 0},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::SLO, AddressMode::IZX, EXEC_OF(slo), 8, 0},
    {OpcodeType::NOP, AddressMode::ZP0, EXEC_OF(nop), 3, 0},
    {OpcodeType::ORA, AddressMode::ZP0, EXEC_OF(ora), 3, 0},
    {OpcodeType::ASL, AddressMode::ZP0, EXEC_OF(asl), 5, 0},
    {OpcodeType::SLO, AddressMode::ZP0, EXEC_OF(slo), 5, 0},
    {OpcodeType::PHP, AddressMode::IMP, EXEC_OF(php), 3, 0},
    {OpcodeType::ORA, AddressMode::IMM, EXEC_OF(ora), 2, 0},
    {OpcodeType::ASL, AddressMode::ACC, EXEC_OF(asl), 2, 0},
    {OpcodeType::ANC, AddressMode::IMM, EXEC_OF(anc), 2, 0},
    {OpcodeType::NOP, AddressMode::ABS, EXEC_OF(nop), 4, 0},
    {OpcodeType::ORA, AddressMode::ABS, EXEC_OF(ora), 4, 0},
    {OpcodeType::ASL, AddressMode::ABS, EXEC_OF(asl), 6, 0},
    {OpcodeType::SLO, AddressMode::ABS, EXEC_OF(slo), 6, 0},
    // -- 1x
    {OpcodeType::BPL, AddressMode::REL, EXEC_OF(bpl), 2, 0},
    {OpcodeType::ORA, AddressMode::IZY, EXEC_OF(ora), 5, 1},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::SLO, AddressMode::IZY, EXEC_OF(slo), 8, 0},
    {OpcodeType::NOP, AddressMode::ZPX, EXEC_OF(nop), 4, 0},
    {OpcodeType::ORA, AddressMode::ZPX, EXEC_OF(ora), 4, 0},
    {OpcodeType::ASL, AddressMode::ZPX, EXEC_OF(asl), 6, 0},
    {OpcodeType::SLO, AddressMode::ZPX, EXEC_OF(slo), 6, 0},
    {OpcodeType::CLC, AddressMode::IMP, EXEC_OF(clc), 2, 0},
    {OpcodeType::ORA, AddressMode::ABY, EXEC_OF(ora), 4, 1},
    {OpcodeType::NOP, AddressMode::IMP, EXEC_OF(nop), 2, 0},
    {OpcodeType::SLO, AddressMode::ABY, EXEC_OF(slo), 7, 0},
    {OpcodeType::NOP, AddressMode::ABX, EXEC_OF(nop), 4, 1},
    {OpcodeType::ORA, AddressMode::ABX, EXEC_OF(ora), 4, 1},
    {OpcodeType::ASL, AddressMode::ABX, EXEC_OF(asl), 7, 0},
    {OpcodeType::SLO, AddressMode::ABX, EXEC_OF(slo), 7, 0},
    // -- 2x
    {OpcodeType::JSR, AddressMode::ABS, EXEC_OF(jsr), 6, 0},
    {OpcodeType::AND, AddressMode::IZX, EXEC_OF(and), 6, 0},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::RLA, AddressMode::IZX, EXEC_OF(rla), 8, 0},
    {OpcodeType::BIT, AddressMode::ZP0, EXEC_OF(bit), 3, 0},
    {OpcodeType::AND, AddressMode::ZP0, EXEC_OF(and), 3, 0},
    {OpcodeType::ROL, AddressMode::ZP0, EXEC_OF(rol), 5, 0},
    {OpcodeType::RLA, AddressMode::ZP0, EXEC_OF(rla), 5, 0},
    {OpcodeType::PLP, AddressMode::IMP, EXEC_OF(plp), 4, 0},
    {OpcodeType::AND, AddressMode::IMM, EXEC_OF(and), 2, 0},
    {OpcodeType::ROL, AddressMode::ACC, EXEC_OF(rol), 2, 0},
    {OpcodeType::ANC, AddressMode::IMM, EXEC_OF(anc), 2, 0},
    {OpcodeType::BIT, AddressMode::ABS, EXEC_OF(bit), 4, 0},
    {OpcodeType::AND, AddressMode::ABS, EXEC_OF(and), 4, 0},
    {OpcodeType::ROL, AddressMode::ABS, EXEC_OF(rol), 6, 0},
    {OpcodeType::RLA, AddressMode::ABS, EXEC_OF(rla), 6, 0},
    // -- 3x
    {OpcodeType::BMI, AddressMode::REL, EXEC_OF(bmi), 2, 0},
    {OpcodeType::AND, AddressMode::IZY, EXEC_OF(and), 5, 1},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::RLA, AddressMode::IZY, EXEC_OF(rla), 8, 0},
    {OpcodeType::NOP, AddressMode::ZPX, EXEC_OF(nop), 4, 0},
    {OpcodeType::AND, AddressMode::ZPX, EXEC_OF(and), 4, 0},
    {OpcodeType::ROL, AddressMode::ZPX, EXEC_OF(rol), 6, 0},
    {OpcodeType::RLA, AddressMode::ZPX, EXEC_OF(rla), 6, 0},
    {OpcodeType::SEC, AddressMode::IMP, EXEC_OF(sec), 2, 0},
    {OpcodeType::AND, AddressMode::ABY, EXEC_OF(and), 4, 1},
    {OpcodeType::NOP, AddressMode::IMP, EXEC_OF(nop), 2, 0},
    {OpcodeType::RLA, AddressMode::ABY, EXEC_OF(rla), 7, 0},
    {OpcodeType::NOP, AddressMode::ABX, EXEC_OF(nop), 4, 1},
    {OpcodeType::AND, AddressMode::ABX, EXEC_OF(and), 4, 1},
    {OpcodeType::ROL, AddressMode::ABX, EXEC_OF(rol), 7, 0},
    {OpcodeType::RLA, AddressMode::ABX, EXEC_OF(rla), 7, 0},
    // -- 4x
    {OpcodeType::RTI, AddressMode::IMP, EXEC_OF(rti), 6, 0},
    {OpcodeType::EOR, AddressMode::IZX, EXEC_OF(eor), 6, 0},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::SRE, AddressMode::IZX, EXEC_OF(sre), 8, 0},
    {OpcodeType::NOP, AddressMode::ZP0, EXEC_OF(nop), 3, 0},
    {OpcodeType::EOR, AddressMode::ZP0, EXEC_OF(eor), 3, 0},
    {OpcodeType::LSR, AddressMode::ZP0, EXEC_OF(lsr), 5, 0},
    {OpcodeType::SRE, AddressMode::ZP0, EXEC_OF(sre), 5, 0},
    {OpcodeType::PHA, AddressMode::IMP, EXEC_OF(pha), 3, 0},
    {OpcodeType::EOR, AddressMode::IMM, EXEC_OF(eor), 2, 0},
    {OpcodeType::LSR, AddressMode::ACC, EXEC_OF(lsr), 2, 0},
    {OpcodeType::ALR, AddressMode::IMM, EXEC_OF(alr), 2, 0},
    {OpcodeType::JMP, AddressMode::ABS, EXEC_OF(jmp), 3, 0},
    {OpcodeType::EOR, AddressMode::ABS, EXEC_OF(eor), 4, 0},
    {OpcodeType::LSR, AddressMode::ABS, EXEC_OF(lsr), 6, 0},
    {OpcodeType::SRE, AddressMode::ABS, EXEC_OF(sre), 6, 0},
    // -- 5x
    {OpcodeType::BVC, AddressMode::REL, EXEC_OF(bvc), 2, 0},
    {OpcodeType::EOR, AddressMode::IZY, EXEC_OF(eor), 5, 1},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::SRE, AddressMode::IZY, EXEC_OF(sre), 8, 0},
    {OpcodeType::NOP, AddressMode::ZPX, EXEC_OF(nop), 4, 0},
    {OpcodeType::EOR, AddressMode::ZPX, EXEC_OF(eor), 4, 0},
    {OpcodeType::LSR, AddressMode::ZPX, EXEC_OF(lsr), 6, 0},
    {OpcodeType::SRE, AddressMode::ZPX, EXEC_OF(sre), 6, 0},
    {OpcodeType::CLI, AddressMode::IMP, EXEC_OF(cli), 2, 0},
    {OpcodeType::EOR, AddressMode::ABY, EXEC_OF(eor), 4, 1},
    {OpcodeType::NOP, AddressMode::IMP, EXEC_OF(nop), 2, 0},
    {OpcodeType::SRE, AddressMode::ABY, EXEC_OF(sre), 7, 0},
    {OpcodeType::NOP, AddressMode::ABX, EXEC_OF(nop), 4, 1},
    {OpcodeType::EOR, AddressMode::ABX, EXEC_OF(eor), 4, 1},
    {OpcodeType::LSR, AddressMode::ABX, EXEC_OF(lsr), 7, 0},
    {OpcodeType::SRE, AddressMode::ABX, EXEC_OF(sre), 7, 0},
    // -- 6x
    {OpcodeType::RTS, AddressMode::IMP, EXEC_OF(rts), 6, 0},
    {OpcodeType::ADC, AddressMode::IZX, EXEC_OF(adc), 6, 0},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::RRA, AddressMode::IZX, EXEC_OF(rra), 8, 0},
    {OpcodeType::NOP, AddressMode::ZP0, EXEC_OF(nop), 3, 0},
    {OpcodeType::ADC, AddressMode::ZP0, EXEC_OF(adc), 3, 0},
    {OpcodeType::ROR, AddressMode::ZP0, EXEC_OF(ror), 5, 0},
    {OpcodeType::RRA, AddressMode::ZP0, EXEC_OF(rra), 5, 0},
    {OpcodeType::PLA, AddressMode::IMP, EXEC_OF(pla), 4, 0},
    {OpcodeType::ADC, AddressMode::IMM, EXEC_OF(adc), 2, 0},
    {OpcodeType::ROR, AddressMode::ACC, EXEC_OF(ror), 2, 0},
    {OpcodeType::ARR, AddressMode::IMM, EXEC_OF(arr), 2, 0},
    {OpcodeType::JMP, AddressMode::IND, EXEC_OF(jmp), 5, 0},
    {OpcodeType::ADC, AddressMode::ABS, EXEC_OF(adc), 4, 0},
    {OpcodeType::ROR, AddressMode::ABS, EXEC_OF(ror), 6, 0},
    {OpcodeType::RRA, AddressMode::ABS, EXEC_OF(rra), 6, 0},
    // -- 7x
    {OpcodeType::BVS, AddressMode::REL, EXEC_OF(bvs), 2, 0},
    {OpcodeType::ADC, AddressMode::IZY, EXEC_OF(adc), 5, 1},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::RRA, AddressMode::IZY, EXEC_OF(rra), 8, 0},
    {OpcodeType::NOP, AddressMode::ZPX, EXEC_OF(nop), 4, 0},
    {OpcodeType::ADC, AddressMode::ZPX, EXEC_OF(adc), 4, 0},
    {OpcodeType::ROR, AddressMode::ZPX, EXEC_OF(ror), 6, 0},
    {OpcodeType::RRA, AddressMode::ZPX, EXEC_OF(rra), 6, 0},
    {OpcodeType::SEI, AddressMode::IMP, EXEC_OF(sei), 2, 0},
    {OpcodeType::ADC, AddressMode::ABY, EXEC_OF(adc), 4, 1},
    {OpcodeType::NOP, AddressMode::IMP, EXEC_OF(nop), 2, 0},
    {OpcodeType::RRA, AddressMode::ABY, EXEC_OF(rra), 7, 0},
    {OpcodeType::NOP, AddressMode::ABX, EXEC_OF(nop), 4, 1},
    {OpcodeType::ADC, AddressMode::ABX, EXEC_OF(adc), 4, 1},
    {OpcodeType::ROR, AddressMode::ABX, EXEC_OF(ror), 7, 0},
    {OpcodeType::RRA, AddressMode::ABX, EXEC_OF(rra), 7, 0},
    // -- 8x
    {OpcodeType::NOP, AddressMode::IMM, EXEC_OF(nop), 2, 0},
    {OpcodeType::STA, AddressMode::IZX, EXEC_OF(sta), 6, 0},
    {OpcodeType::NOP, AddressMode::IMM, EXEC_OF(nop), 2, 0},
    {OpcodeType::SAX, AddressMode::IZX, EXEC_OF(sax), 6, 0},
    {OpcodeType::STY, AddressMode::ZP0, EXEC_OF(sty), 3, 0},
    {OpcodeType::STA, AddressMode::ZP0, EXEC_OF(sta), 3, 0},
    {OpcodeType::STX, AddressMode::ZP0, EXEC_OF(stx), 3, 0},
    {OpcodeType::SAX, AddressMode::ZP0, EXEC_OF(sax), 3, 0},
    {OpcodeType::DEY, AddressMode::IMP, EXEC_OF(dey), 2, 0},
    {OpcodeType::NOP, AddressMode::IMM, EXEC_OF(nop), 2, 0},
    {OpcodeType::TXA, AddressMode::IMP, EXEC_OF(txa), 2, 0},
    {OpcodeType::XAA, AddressMode::IMM, EXEC_OF(xaa), 2, 0},
    {OpcodeType::STY, AddressMode::ABS, EXEC_OF(sty), 4, 0},
    {OpcodeType::STA, AddressMode::ABS, EXEC_OF(sta), 4, 0},
    {OpcodeType::STX, AddressMode::ABS, EXEC_OF(stx), 4, 0},
    {OpcodeType::SAX, AddressMode::ABS, EXEC_OF(sax), 4, 0},
    // -- 9x
    {OpcodeType::BCC, AddressMode::REL, EXEC_OF(bcc), 2, 0},
    {OpcodeType::STA, AddressMode::IZY, EXEC_OF(sta), 6, 0},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::AHX, AddressMode::IZY, EXEC_OF(ahx), 6, 0},
    {OpcodeType::STY, AddressMode::ZPX, EXEC_OF(sty), 4, 0},
    {OpcodeType::STA, AddressMode::ZPX, EXEC_OF(sta), 4, 0},
    {OpcodeType::STX, AddressMode::ZPY, EXEC_OF(stx), 4, 0},
    {OpcodeType::SAX, AddressMode::ZPY, EXEC_OF(sax), 4, 0},
    {OpcodeType::TYA, AddressMode::IMP, EXEC_OF(tya), 2, 0},
    {OpcodeType::STA, AddressMode::ABY, EXEC_OF(sta), 5, 0},
    {OpcodeType::TXS, AddressMode::IMP, EXEC_OF(txs), 2, 0},
    {OpcodeType::TAS, AddressMode::ABY, EXEC_OF(tas), 5, 0},
    {OpcodeType::SHY, AddressMode::ABX, EXEC_OF(shy), 5, 0},
    {OpcodeType::STA, AddressMode::ABX, EXEC_OF(sta), 5, 0},
    {OpcodeType::SHX, AddressMode::ABY, EXEC_OF(shx), 5, 0},
    {OpcodeType::AHX, AddressMode::ABY, EXEC_OF(ahx), 5, 0},
    // -- Ax
    {OpcodeType::LDY, AddressMode::IMM, EXEC_OF(ldy), 2, 0},
    {OpcodeType::LDA, AddressMode::IZX, EXEC_OF(lda), 6, 0},
    {OpcodeType::LDX, AddressMode::IMM, EXEC_OF(ldx), 2, 0},
    {OpcodeType::LAX, AddressMode::IZX, EXEC_OF(lax), 6, 0},
    {OpcodeType::LDY, AddressMode::ZP0, EXEC_OF(ldy), 3, 0},
    {OpcodeType::LDA, AddressMode::ZP0, EXEC_OF(lda), 3, 0},
    {OpcodeType::LDX, AddressMode::ZP0, EXEC_OF(ldx), 3, 0},
    {OpcodeType::LAX, AddressMode::ZP0, EXEC_OF(lax), 3, 0},
    {OpcodeType::TAY, AddressMode::IMP, EXEC_OF(tay), 2, 0},
    {OpcodeType::LDA, AddressMode::IMM, EXEC_OF(lda), 2, 0},
    {OpcodeType::TAX, AddressMode::IMP, EXEC_OF(tax), 2, 0},
    {OpcodeType::LAX, AddressMode::IMM, EXEC_OF(lax), 2, 0},
    {OpcodeType::LDY, AddressMode::ABS, EXEC_OF(ldy), 4, 0},
    {OpcodeType::LDA, AddressMode::ABS, EXEC_OF(lda), 4, 0},
    {OpcodeType::LDX, AddressMode::ABS, EXEC_OF(ldx), 4, 0},
    {OpcodeType::LAX, AddressMode::ABS, EXEC_OF(lax), 4, 0},
    // -- Bx
    {OpcodeType::BCS, AddressMode::REL, EXEC_OF(bcs), 2, 0},
    {OpcodeType::LDA, AddressMode::IZY, EXEC_OF(lda), 5, 1},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::LAX, AddressMode::IZY, EXEC_OF(lax), 5, 1},
    {OpcodeType::LDY, AddressMode::ZPX, EXEC_OF(ldy), 4, 0},
    {OpcodeType::LDA, AddressMode::ZPX, EXEC_OF(lda), 4, 0},
    {OpcodeType::LDX, AddressMode::ZPY, EXEC_OF(ldx), 4, 0},
    {OpcodeType::LAX, AddressMode::ZPY, EXEC_OF(lax), 4, 0},
    {OpcodeType::CLV, AddressMode::IMP, EXEC_OF(clv), 2, 0},
    {OpcodeType::LDA, AddressMode::ABY, EXEC_OF(lda), 4, 1},
    {OpcodeType::TSX, AddressMode::IMP, EXEC_OF(tsx), 2, 0},
    {OpcodeType::LAS, AddressMode::ABY, EXEC_OF(las), 4, 1},
    {OpcodeType::LDY, AddressMode::ABX, EXEC_OF(ldy), 4, 1},
    {OpcodeType::LDA, AddressMode::ABX, EXEC_OF(lda), 4, 1},
    {OpcodeType::LDX, AddressMode::ABY, EXEC_OF(ldx), 4, 1},
    {OpcodeType::LAX, AddressMode::ABY, EXEC_OF(lax), 4, 1},
    // -- Cx
    {OpcodeType::CPY, AddressMode::IMM, EXEC_OF(cpy), 2, 0},
    {OpcodeType::CMP, AddressMode::IZX, EXEC_OF(cmp), 6, 0},
    {OpcodeType::NOP, AddressMode::IMM, EXEC_OF(nop), 2, 0},
    {OpcodeType::DCP, AddressMode::IZX, EXEC_OF(dcp), 8, 0},
    {OpcodeType::CPY, AddressMode::ZP0, EXEC_OF(cpy), 3, 0},
    {OpcodeType::CMP, AddressMode::ZP0, EXEC_OF(cmp), 3, 0},
    {OpcodeType::DEC, AddressMode::ZP0, EXEC_OF(dec), 5, 0},
    {OpcodeType::DCP, AddressMode::ZP0, EXEC_OF(dcp), 5, 0},
    {OpcodeType::INY, AddressMode::IMP, EXEC_OF(iny), 2, 0},
    {OpcodeType::CMP, AddressMode::IMM, EXEC_OF(cmp), 2, 0},
    {OpcodeType::DEX, AddressMode::IMP, EXEC_OF(dex), 2, 0},
    {OpcodeType::AXS, AddressMode::IMM, EXEC_OF(axs), 2, 0},
    {OpcodeType::CPY, AddressMode::ABS, EXEC_OF(cpy), 4, 0},
    {OpcodeType::CMP, AddressMode::ABS, EXEC_OF(cmp), 4, 0},
    {OpcodeType::DEC, AddressMode::ABS, EXEC_OF(dec), 6, 0},
    {OpcodeType::DCP, AddressMode::ABS, EXEC_OF(dcp), 6, 0},
    // -- Dx
    {OpcodeType::BNE, AddressMode::REL, EXEC_OF(bne), 2, 0},
    {OpcodeType::CMP, AddressMode::IZY, EXEC_OF(cmp), 5, 1},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::DCP, AddressMode::IZY, EXEC_OF(dcp), 8, 0},
    {OpcodeType::NOP, AddressMode::ZPX, EXEC_OF(nop), 4, 0},
    {OpcodeType::CMP, AddressMode::ZPX, EXEC_OF(cmp), 4, 0},
    {OpcodeType::DEC, AddressMode::ZPX, EXEC_OF(dec), 6, 0},
    {OpcodeType::DCP, AddressMode::ZPX, EXEC_OF(dcp), 6, 0},
    {OpcodeType::CLD, AddressMode::IMP, EXEC_OF(cld), 2, 0},
    {OpcodeType::CMP, AddressMode::ABY, EXEC_OF(cmp), 4, 1},
    {OpcodeType::NOP, AddressMode::IMP, EXEC_OF(nop), 2, 0},
    {OpcodeType::DCP, AddressMode::ABY, EXEC_OF(dcp), 7, 0},
    {OpcodeType::NOP, AddressMode::ABX, EXEC_OF(nop), 4, 1},
    {OpcodeType::CMP, AddressMode::ABX, EXEC_OF(cmp), 4, 1},
    {OpcodeType::DEC, AddressMode::ABX, EXEC_OF(dec), 7, 0},
    {OpcodeType::DCP, AddressMode::ABX, EXEC_OF(dcp), 7, 0},
    // -- Ex
    {OpcodeType::CPX, AddressMode::IMM, EXEC_OF(cpx), 2, 0},
    {OpcodeType::SBC, AddressMode::IZX, EXEC_OF(sbc), 6, 0},
    {OpcodeType::NOP, AddressMode::IMM, EXEC_OF(nop), 2, 0},
    {OpcodeType::ISC, AddressMode::IZX, EXEC_OF(isc), 8, 0},
    {OpcodeType::CPX, AddressMode::ZP0, EXEC_OF(cpx), 3, 0},
    {OpcodeType::SBC, AddressMode::ZP0, EXEC_OF(sbc), 3, 0},
    {OpcodeType::INC, AddressMode::ZP0, EXEC_OF(inc), 5, 0},
    {OpcodeType::ISC, AddressMode::ZP0, EXEC_OF(isc), 5, 0},
    {OpcodeType::INX, AddressMode::IMP, EXEC_OF(inx), 2, 0},
    {OpcodeType::SBC, AddressMode::IMM, EXEC_OF(sbc), 2, 0},
    {OpcodeType::NOP, AddressMode::IMP, EXEC_OF(nop), 2, 0},
    {OpcodeType::SBC, AddressMode::IMM, EXEC_OF(sbc), 2, 0},
    {OpcodeType::CPX, AddressMode::ABS, EXEC_OF(cpx), 4, 0},
    {OpcodeType::SBC, AddressMode::ABS, EXEC_OF(sbc), 4, 0},
    {OpcodeType::INC, AddressMode::ABS, EXEC_OF(inc), 6, 0},
    {OpcodeType::ISC, AddressMode::ABS, EXEC_OF(isc), 6, 0},
    // -- Fx
    {OpcodeType::BEQ, AddressMode::REL, EXEC_OF(beq), 2, 0},
    {OpcodeType::SBC, AddressMode::IZY, EXEC_OF(sbc), 5, 1},
    {OpcodeType::KIL, AddressMode::IMP, EXEC_OF(kil), 0, 0},
    {OpcodeType::ISC, AddressMode::IZY, EXEC_OF(isc), 8, 0},
    {OpcodeType::NOP, AddressMode::ZPX, EXEC_OF(nop), 4, 0},
    {OpcodeType::SBC, AddressMode::ZPX, EXEC_OF(sbc), 4, 0},
    {OpcodeType::INC, AddressMode::ZPX, EXEC_OF(inc), 6, 0},
    {OpcodeType::ISC, AddressMode::ZPX, EXEC_OF(isc), 6, 0},
    {OpcodeType::SED, AddressMode::IMP, EXEC_OF(sed), 2, 0},
    {OpcodeType::SBC, AddressMode::ABY, EXEC_OF(sbc), 4, 1},
    {OpcodeType::NOP, AddressMode::IMP, EXEC_OF(nop), 2, 0},
    {OpcodeType::ISC, AddressMode::ABY, EXEC_OF(isc), 7, 0},
    {OpcodeType::NOP, AddressMode::ABX, EXEC_OF(nop), 4, 1},
    {OpcodeType::SBC, AddressMode::ABX, EXEC_OF(sbc), 4, 1},
    {OpcodeType::INC, AddressMode::ABX, EXEC_OF(inc), 7, 0},
    {OpcodeType::ISC, AddressMode::ABX, EXEC_OF(isc), 7, 0},
    /* clang-format on */
};

} // namespace ln
