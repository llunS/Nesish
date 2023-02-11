#include "memory.hpp"

#include <cstring>

#define BASE MappableMemory<MemoryMappingPoint, LN_ADDRESSABLE_SIZE>

namespace ln {

constexpr Address Memory::STACK_PAGE_MASK;

constexpr Address Memory::NMI_VECTOR_ADDR;
constexpr Address Memory::RESET_VECTOR_ADDR;
constexpr Address Memory::IRQ_VECTOR_ADDR;

Memory::Memory()
    : m_ram{}
    , m_read_latch(0)
{
    // Internal RAM space mapping
    {
        auto decode = [](const MappingEntry *i_entry, Address i_addr,
                         Byte *&o_addr) -> ln::Error {
            Byte *ram = (Byte *)i_entry->opaque;

            Address addr = i_addr & LN_RAM_ADDR_MASK;
            o_addr = ram + addr;
            return Error::OK;
        };
        set_mapping(MemoryMappingPoint::INTERNAL_RAM,
                    {LN_RAM_ADDR_HEAD, LN_RAM_ADDR_TAIL, false, decode, m_ram});
    }
}

Error
Memory::get_byte(Address i_addr, Byte &o_val) const
{
    // https://www.nesdev.org/wiki/Open_bus_behavior
    auto err = BASE::get_byte(i_addr, o_val);
    if (!LN_FAILED(err))
    {
        m_read_latch = o_val;
    }
    else
    {
        if (err == Error::UNAVAILABLE || err == Error::WRITE_ONLY)
        {
            o_val = m_read_latch;
            err = Error::OK;
        }
    }
    return err;
}

Byte
Memory::get_latch() const
{
    return m_read_latch;
}

Error
Memory::set_bulk(Address i_begin, Address i_end, Byte i_byte)
{
    if (i_begin >= i_end)
    {
        return Error::INVALID_ARGUMENT;
    }

    // optimize only simple cases for now.
    if (i_begin < LN_INTERNAL_RAM_SIZE && i_end < LN_INTERNAL_RAM_SIZE)
    {
        std::memset(m_ram + i_begin, i_byte, i_end - i_begin);
        return Error::OK;
    }

    // For now, only support consecutive and non-page-crossing range.
    return Error::INVALID_ARGUMENT;
}

} // namespace ln
