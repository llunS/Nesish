#ifndef LN_CONSOLE_CPU_HPP
#define LN_CONSOLE_CPU_HPP

#include "console/memory.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"

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
    Byte A;     // Accumulator
    Byte X;     // Index X
    Byte Y;     // Index Y
    Address PC; // Program Counter
    Byte S;     // Stack Pointer
    Byte P;     // Status

    // ---- External components references
    Memory *m_memory;
};

} // namespace ln

#endif // LN_CONSOLE_CPU_HPP
