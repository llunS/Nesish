
#ifndef LN_CONSOLE_PERIPHERAL_CONTROLLER_HPP
#define LN_CONSOLE_PERIPHERAL_CONTROLLER_HPP

#include <type_traits>

namespace ln {

enum Key {
    BEGIN = 0,

    /* value is the bit position */
    A = 0,
    B = 1,
    SELECT = 2,
    START = 3,
    UP = 4,
    DOWN = 5,
    LEFT = 6,
    RIGHT = 7,

    SIZE,
    END = SIZE,
};
typedef std::underlying_type<ln::Key>::type KeyIt;

struct Controller {
  public:
    virtual ~Controller(){};

    virtual void
    strobe(bool i_on) = 0;
    virtual bool
    report() = 0;
};

} // namespace ln

#endif // LN_CONSOLE_PERIPHERAL_CONTROLLER_HPP
