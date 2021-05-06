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

    inline void einkOn()  { e_ink.turn_on(); }
    inline void einkOff() { e_ink.turn_off(); }
    inline uint8_t getPanelState() { return (uint8_t) e_ink.get_panel_state(); }

    inline double readBattery() { return battery.read_level(); }

    uint8_t readPowerGood() { return e_ink.read_power_good(); }

    int8_t readTemperature() { return e_ink.read_temperature(); }

    #if defined(EXTENDED_CASE)
      uint8_t readPresskey(int c) { return press_keys.read_key((PressKeys::Key) c); }
    #else
      #if defined(INKPLATE_6) || defined(INKPLATE_10)
        uint8_t readTouchpad(int c) { return touch_keys.read_key((TouchKeys::Key) c); }
      #endif
    #endif
    
    inline void  disconnect() { network_client.disconnect(); }
    inline bool isConnected() { return network_client.isConnected(); }
    inline int _getRotation() { return Graphics::getRotation(); }

    inline bool joinAP(const char * ssid, const char * pass) { 
                return network_client.joinAP(ssid, pass); }

    bool sdCardInit() { return true; }
};

