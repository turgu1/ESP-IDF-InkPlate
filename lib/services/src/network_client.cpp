/*
NetworkClient.cpp
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

#define __NETWORK_CLIENT__ 1
#include "network_client.hpp"

bool 
NetworkClient::joinAP(const char * ssid, const char * pass)
{
  return true;
}

void 
NetworkClient::disconnect()
{
}

bool 
NetworkClient::isConnected()
{
  return true;
}


uint8_t *
NetworkClient::downloadFile(const char * url, int32_t * defaultLen)
{
  return nullptr;
}
