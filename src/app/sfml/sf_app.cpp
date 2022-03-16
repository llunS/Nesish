#include "sf_app.hpp"

#include "console/emulator.hpp"
#include "console/spec.hpp"

#include "app/sfml/sf_controller.hpp"

#include "SFML/Window.hpp"

#define RESOLUTION_SCALE 6

namespace ln {

int
SFApp::run()
{
    sf::Window window(sf::VideoMode(RESOLUTION_SCALE * LN_NES_WIDTH,
                                    RESOLUTION_SCALE * LN_NES_HEIGHT),
                      "LightNES");

    // we‘ll do the timing ourselves, so we don't want to be confined by vsync.
    window.setVerticalSyncEnabled(false);

    ln::Emulator emulator;
    emulator.plug_controller(ln::Emulator::P1, new ln::SFController());
    emulator.power_up();

    // run the program as long as the window is open
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last
        // iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // check the type of the event...
            switch (event.type)
            {
                // window closed
                case sf::Event::Closed:
                    window.close();
                    break;

                // we don't process other types of events
                default:
                    break;
            }
        }
    }

    return 0;
}

} // namespace ln
