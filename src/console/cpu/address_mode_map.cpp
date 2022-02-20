#include "console/cpu/cpu.hpp"
#include "console/cpu/address_mode_parse.hpp"

#define PARSE_OF(i_address_mode_name)                                          \
    CPU::AddressModeParse::parse_##i_address_mode_name

namespace ln {

CPU::AddressModeParseEntry CPU::s_address_mode_map[AddressMode::COUNT] = {
    // http://www.oxyron.de/html/opcodes02.html
    /* clang-format off */
    {AddressMode::IMP, PARSE_OF(imp)},
    {AddressMode::ACC, PARSE_OF(acc)},
    {AddressMode::IMM, PARSE_OF(imm)},
    {AddressMode::ZP0, PARSE_OF(zp0)},
    {AddressMode::ZPX, PARSE_OF(zpx)},
    {AddressMode::ZPY, PARSE_OF(zpy)},
    {AddressMode::IZX, PARSE_OF(izx)},
    {AddressMode::IZY, PARSE_OF(izy)},
    {AddressMode::ABS, PARSE_OF(abs)},
    {AddressMode::ABX, PARSE_OF(abx)},
    {AddressMode::ABY, PARSE_OF(aby)},
    {AddressMode::IND, PARSE_OF(ind)},
    {AddressMode::REL, PARSE_OF(rel)},
    /* clang-format on */
};

} // namespace ln
