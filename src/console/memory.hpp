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

  public:
    friend struct CPU;

  private:
    Byte
    get_byte(Address i_address) const;
    void
    set_byte(Address i_address, Byte i_byte);

    void
    set_range(Address i_begin, Address i_end, Byte i_byte);

  private:
    static constexpr Address APU_FC_ADDRESS = 0x4017;
    void
    set_apu_frame_counter(Byte i_byte);

  private:
    // ---- Memory Map
    // https://wiki.nesdev.org/w/index.php?title=CPU_memory_map
    Byte m_ram[LN_RAM_SIZE];
    // @TODO: Handle special area mappings.
};

} // namespace ln

#endif // LN_CONSOLE_MEMORY_HPP
