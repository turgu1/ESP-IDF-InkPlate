/*
network_client.hpp

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

#include <cstdint>

class NetworkClient
{
  public:
    NetworkClient() : connected(false) {}

    bool joinAP(const char * ssid, const char * pass);
    void disconnect();
    void forceDisconnect();

    inline bool isConnected() { return connected; }

    uint8_t * downloadFile(const char * url, int32_t * defaultLen);

  private:
    bool connected;
};

#if __NETWORK_CLIENT__
  NetworkClient network_client;
#else
  extern NetworkClient network_client;
#endif
