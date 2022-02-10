#ifndef LN_CONSOLE_CPU_HPP
#define LN_CONSOLE_CPU_HPP

#include <cstdint>

#include "console/memory.hpp"
#include "common/klass.hpp"

namespace ln {

struct CPU {
  public:
    CPU(Memory *i_memory);
    LN_KLZ_DELETE_COPY_MOVE(CPU);

  public:
    struct OpCodeExec;
    struct OperandGet;

  private:
    // ---- Registers
    // https://wiki.nesdev.org/w/index.php?title=CPU_registers
    uint8_t A;   // Accumulator
    uint8_t X;   // Index X
    uint8_t Y;   // Index Y
    uint16_t PC; // Program Counter
    uint8_t S;   // Stack Pointer
    uint8_t P;   // Status

    // ---- External components references
    Memory *m_memory;
};

} // namespace ln

#endif // LN_CONSOLE_CPU_HPP
