
#pragma once

#include <type_traits>

namespace ln {

enum Key {
    KEY_BEGIN = 0,

    /* value is the bit position */
    KEY_A = 0,
    KEY_B = 1,
    KEY_SELECT = 2,
    KEY_START = 3,
    KEY_UP = 4,
    KEY_DOWN = 5,
    KEY_LEFT = 6,
    KEY_RIGHT = 7,

    KEY_SIZE,
    KEY_END = KEY_SIZE,
};
typedef std::underlying_type<ln::Key>::type KeyIt;

struct Controller {
  public:
    virtual ~Controller() = default;

    virtual void
    strobe(bool i_on) = 0;
    virtual bool
    report() = 0;
};

} // namespace ln
