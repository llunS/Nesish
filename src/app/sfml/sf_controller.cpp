#include "sf_controller.hpp"

#include "SFML/Window/Keyboard.hpp"

#include <type_traits>

namespace ln {

static sf::Keyboard::Key
pvt_map_key(Key i_key);

SFController::SFController()
    : m_strobe_idx(Key::BEGIN)
{
}

void
SFController::strobe(bool i_on)
{
    if (i_on)
    {
        // reload all bits with latest state.
        for (std::underlying_type<Key>::type it = Key::BEGIN; it < Key::END;
             ++it)
        {
            auto key = Key(it);

            auto sf_key = pvt_map_key(key);
            m_key_state[key] = sf::Keyboard::isKeyPressed(sf_key);
        }
    }
    else
    {
        m_strobe_idx = Key::BEGIN;
    }
}

bool
SFController::report()
{
    if (m_strobe_idx < Key::END)
    {
        return m_key_state[m_strobe_idx];
        ++m_strobe_idx;
    }
    else
    {
        // https://www.nesdev.org/wiki/Standard_controller
        // "All subsequent reads will return 1 on official Nintendo brand
        // controllers but may return 0 on third party controllers such as the
        // U-Force."
        return true;
    }
}

sf::Keyboard::Key
pvt_map_key(Key i_key)
{
    // @TODO: Support custom mapping.
    constexpr static sf::Keyboard::Key s_mapping[Key::SIZE] = {
        sf::Keyboard::K, sf::Keyboard::J, sf::Keyboard::V, sf::Keyboard::B,
        sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D,
    };
    return s_mapping[i_key];
}

} // namespace ln
