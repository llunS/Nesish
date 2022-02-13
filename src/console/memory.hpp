#ifndef LN_CONSOLE_MEMORY_HPP
#define LN_CONSOLE_MEMORY_HPP

#include "console/spec.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"

namespace ln {

struct Memory {
  public:
    Memory();
    LN_KLZ_DELETE_COPY_MOVE(Memory);

    Byte
    get_byte(Address i_address) const;
    void
    set_byte(Address i_address, Byte i_byte);

  public:
    friend struct CPU;

  private:
    // ---- Memory Map
    // https://wiki.nesdev.org/w/index.php?title=CPU_memory_map
    Byte m_ram[LN_INTERNAL_RAM_SIZE];
    // @TODO: Handle other addressable areas.
};

} // namespace ln

#endif // LN_CONSOLE_MEMORY_HPP
