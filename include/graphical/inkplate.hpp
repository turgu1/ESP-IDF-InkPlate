/*
inkplate.hpp
Inkplate 6 Arduino library
David Zovko, Borna Biro, Denis Vajak, Zvonimir Haramustek @ e-radionica.com
September 24, 2020
https://github.com/e-radionicacom/Inkplate-6-Arduino-library

For support, please reach over forums: forum.e-radionica.com/en
For more info about the product, please check: www.inkplate.io

This code is released under the GNU Lesser General Public License v3.0: https://www.gnu.org/licenses/lgpl-3.0.en.html
Please review the LICENSE file included with this example.
If you have any questions about licensing, please contact techsupport@e-radionica.com
Distributed as-is; no warranty is given.
*/

#pragma once

#include "defines.hpp"

#include "graphics.hpp"
#include "inkplate_platform.hpp"
#include "network_client.hpp"

class Inkplate : public Graphics
{
  public:

    Inkplate(DisplayMode mode);

    void begin() { inkplate_platform.setup(); }

    inline void             einkOn() { e_ink.turn_on();                          }
    inline void            einkOff() { e_ink.turn_off();                         }
    inline uint8_t   getPanelState() { return (uint8_t) e_ink.get_panel_state(); }
    inline double      readBattery() { return battery.read_level();              }
    inline uint8_t   readPowerGood() { return e_ink.read_power_good();           }
    inline int8_t  readTemperature() { return e_ink.read_temperature();          }
    inline void         disconnect() { network_client.disconnect();              }
    inline bool        isConnected() { return network_client.isConnected();      }
    inline int        _getRotation() { return Graphics::getRotation();           }
    inline bool         sdCardInit() { return true;                              }

    inline bool             joinAP(const char * ssid, const char * pass) { 
                                       return network_client.joinAP(ssid, pass); }

    #if defined(EXTENDED_CASE) && (defined(INKPLATE_6) || defined(INKPLATE_10))
      inline uint8_t readPresskey(int c) { return press_keys.read_key((PressKeys::Key) c); }
    #elif defined(INKPLATE_6) || defined(INKPLATE_10)
      inline uint8_t readTouchpad(int c) { return touch_keys.read_key((TouchKeys::Key) c); }
    #elif defined(INKPLATE_6PLUS)
      inline bool touchInArea(int16_t x1, int16_t y1, int16_t w, int16_t h);

      inline bool    tsAvailable() { return touch_screen.is_screen_touched(); }      
      inline uint8_t tsGetData(TouchScreen::TouchPositions & xPos, TouchScreen::TouchPositions & yPos) { return touch_screen.get_positions(xPos, yPos); }
      inline uint8_t tsGetPowerState() { return touch_screen.get_power_state(); }
      inline void    tsSetPowerState(uint8_t s) { touch_screen.set_power_state(s != 0); }
      inline void    tsShutdown();
    #endif
};

