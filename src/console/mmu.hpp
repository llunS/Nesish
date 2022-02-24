#ifndef LN_CONSOLE_MMU_HPP
#define LN_CONSOLE_MMU_HPP

#include <unordered_map>

#include "console/spec.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"
#include "common/error.hpp"
#include "common/hash.hpp"
#include "console/dllexport.h"

namespace ln {

enum MappingPoint : unsigned char {
    INVALID = 0,
    ADHOC, // @TMP

    INTERNAL_RAM,
    PRG_ROM,
    PRG_RAM,
};

struct MappingEntry;
typedef Byte *(*MappingDecodeFunc)(const MappingEntry *i_entry, Address i_addr);
struct MappingEntry {
    Address begin;
    Address end; // inclusive
    bool readonly;

    MappingDecodeFunc decode;
    void *opaque;

    MappingEntry(Address i_begin, Address i_end, bool i_readonly,
                 MappingDecodeFunc i_decode, void *i_opaque);
    MappingEntry(const MappingEntry &) = default; // trivially copyable
};

struct LN_CONSOLE_API MMU {
  public:
    MMU();
    LN_KLZ_DELETE_COPY_MOVE(MMU);

  public:
    static constexpr Address STACK_PAGE_MASK =
        0x0100; // stack page: $0100-$01FF
    static constexpr Address IRQ_VECTOR_ADDR = 0xFFFE; // IRQ/BRK vector

  public:
    void
    set_mapping(MappingPoint i_point, MappingEntry i_entry);
    void
    unset_mapping(MappingPoint i_point);

    Error
    get_byte(Address i_addr, Byte &o_byte) const;
    Error
    set_byte(Address i_addr, Byte i_byte);

    Error
    set_bulk(Address i_begin, Address i_end, Byte i_byte);

  private:
    Byte *
    decode_addr(Address i_addr, bool i_write) const;

  private:
    // ---- Memory Map
    // https://wiki.nesdev.org/w/index.php?title=CPU_memory_map

    // On-board RAM
    Byte m_ram[LN_INTERNAL_RAM_SIZE];
    // @TODO: Handle other mappings
    // @TMP: Adhoc area for NES APU and I/O registers
    Byte m_adhoc[0x0018];

  private:
    MappingPoint m_mapping_registry[LN_ADDRESSABLE_SIZE];
    std::unordered_map<MappingPoint, MappingEntry, EnumHash> m_mapping_entries;
};

} // namespace ln

#endif // LN_CONSOLE_MMU_HPP
