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

#ifndef __INKPLATE_H__
#define __INKPLATE_H__

#include "defines.hpp"

#include "graphics.hpp"
#include "inkplate_platform.hpp"
#include "network_client.hpp"

class Inkplate : public Graphics
{
  public:

    Inkplate(DisplayMode mode);

    void begin(void)       { inkplate_platform.setup(); }

    uint8_t readPowerGood();

    inline bool joinAP(const char * ssid, const char * pass) { return network_client.joinAP(ssid, pass); }
    inline void disconnect()  { network_client.disconnect();         }
    inline bool isConnected() { return network_client.isConnected(); }
    inline int _getRotation() { return Graphics::getRotation();     };
};

#endif
