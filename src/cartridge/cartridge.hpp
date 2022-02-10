#ifndef LN_CARTRIDGE_CARTRIDGE_HPP
#define LN_CARTRIDGE_CARTRIDGE_HPP

#include "common/error.hpp"

namespace ln {

struct Cartridge {
  public:
    virtual ~Cartridge(){};

    virtual Error
    validate() = 0;
};

} // namespace ln

#endif // LN_CARTRIDGE_CARTRIDGE_HPP
