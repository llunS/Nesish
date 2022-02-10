#ifndef LN_CONSOLE_MEMORY_HPP
#define LN_CONSOLE_MEMORY_HPP

#include <cstdint>

#include "console/spec.hpp"
#include "common/klass.hpp"

namespace ln {

struct Memory {
  public:
    Memory();
    LN_KLZ_DELETE_COPY_MOVE(Memory);

  public:
    friend struct CPU;

  private:
    // ---- Memory Map
    // https://wiki.nesdev.org/w/index.php?title=CPU_memory_map
    uint8_t m_ram[LN_INTERNAL_RAM_SIZE];
    // @TODO: Handle other addressable areas.
};

} // namespace ln

#endif // LN_CONSOLE_MEMORY_HPP
