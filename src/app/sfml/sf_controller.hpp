
#ifndef LN_APP_SFML_SFCONTROLLER_HPP
#define LN_APP_SFML_SFCONTROLLER_HPP

#include "console/peripheral/controller.hpp"

namespace ln {

struct SFController : public Controller {
  public:
    SFController();

    void
    strobe(bool i_on) override;
    bool
    report() override;

  private:
    unsigned int m_strobe_idx;
    bool m_key_state[Key::SIZE];
};

} // namespace ln

#endif // LN_APP_SFML_SFCONTROLLER_HPP
