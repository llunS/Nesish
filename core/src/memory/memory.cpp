#include "memory.hpp"

#include <cstring>

#define BASE MappableMemory<MemoryMappingPoint, NH_ADDRESSABLE_SIZE>

namespace nh {

constexpr Address Memory::STACK_PAGE_MASK;

constexpr Address Memory::NMI_VECTOR_ADDR;
constexpr Address Memory::RESET_VECTOR_ADDR;
constexpr Address Memory::IRQ_VECTOR_ADDR;

Memory::Memory(NHLogger *i_logger)
    : BASE(i_logger)
    , m_ram{}
    , m_read_latch(0)
{
    // Internal RAM space mapping
    {
        auto decode = [](const MappingEntry *i_entry, Address i_addr,
                         Byte *&o_addr) -> NHErr {
            Byte *ram = (Byte *)i_entry->opaque;

            Address addr = i_addr & NH_RAM_ADDR_MASK;
            o_addr = ram + addr;
            return NH_ERR_OK;
        };
        set_mapping(MemoryMappingPoint::INTERNAL_RAM,
                    {NH_RAM_ADDR_HEAD, NH_RAM_ADDR_TAIL, false, decode, m_ram});
    }
}

NHErr
Memory::get_byte(Address i_addr, Byte &o_val) const
{
    // https://www.nesdev.org/wiki/Open_bus_behavior
    auto err = BASE::get_byte(i_addr, o_val);
    if (!NH_FAILED(err))
    {
        m_read_latch = o_val;
    }
    else
    {
        if (err == NH_ERR_UNAVAILABLE || err == NH_ERR_WRITE_ONLY)
        {
            o_val = m_read_latch;
            err = NH_ERR_OK;
        }
    }
    return err;
}

Byte
Memory::get_latch() const
{
    return m_read_latch;
}

NHErr
Memory::set_bulk(Address i_begin, Address i_end, Byte i_byte)
{
    if (i_begin >= i_end)
    {
        return NH_ERR_INVALID_ARGUMENT;
    }

    // optimize only simple cases for now.
    if (i_begin < NH_INTERNAL_RAM_SIZE && i_end < NH_INTERNAL_RAM_SIZE)
    {
        std::memset(m_ram + i_begin, i_byte, i_end - i_begin);
        return NH_ERR_OK;
    }

    // For now, only support consecutive and non-page-crossing range.
    return NH_ERR_INVALID_ARGUMENT;
}

} // namespace nh
