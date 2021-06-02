/*
inkplate.cpp
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

#include "inkplate.hpp"
#include "inkplate_platform.hpp"

Inkplate::Inkplate(DisplayMode mode) : 
  Adafruit_GFX(e_ink.get_width(), e_ink.get_height()), 
      Graphics(e_ink.get_width(), e_ink.get_height())
{
  setDisplayMode(mode);
}

#if defined(INKPLATE_6PLUS)

  // The drivers are rotation agnostic. This method is doing proper rotation for touch screen coordinates.
  void 
  Inkplate::rotateFromPhy(TouchScreen::TouchPositions & xPos, TouchScreen::TouchPositions & yPos, uint8_t count)
  {
    uint16_t temp;
    for (int i = 0; i < count; i++) {
      switch (getRotation()) {
      case 0:
        temp = yPos[i];
        yPos[i] = xPos[i];
        xPos[i] = e_ink.get_width() - 1 - temp;
        break;
      case 1:
        // already O.K.
        break;
      case 2:
        temp = yPos[i];
        yPos[i] = e_ink.get_height() - 1 - xPos[i];
        xPos[i] = temp;
        break;
      case 3:
        xPos[i] = e_ink.get_height() - 1 - xPos[i];
        yPos[i] = e_ink.get_width()  - 1 - yPos[i];
        break;
      }
    }    
  }

  /**
   * @brief       touchInArea checks if touch occured in given rectangle area
   * 
   * @param       int16_t x1
   *              rectangle top left corner x plane
   * @param       int16_t y1
   *              rectangle top left corner y plane
   * @param       int16_t w
   *              rectangle width
   * @param       int16_t h
   *              rectangle height
   * 
   * @return      true if successful, false if failed
   */
  bool Inkplate::touchInArea(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h)
  {
    static TouchScreen::TouchPositions touchX, touchY;
    static uint8_t touchN;
    static uint32_t touchT = 0;

    uint16_t x2 = x1 + w, y2 = y1 + h;
    if (tsAvailable())
    {
      uint8_t n;
      TouchScreen::TouchPositions x, y;
      n = tsGetData(x, y);

      if (n)
      {
        touchT = ESP::millis();
        touchN = n;
        touchX = x;
        touchY = y;
      }
    }

    #define BOUND(a, b, c) ((a) <= (b) && (b) <= (c))

    if ((ESP::millis() - touchT) < 100) {
      // Serial.printf("%d: %d, %d - %d, %d\n", touchN, touchX[0], touchY[0], touchX[1], touchY[1]);
      if (touchN == 1 && BOUND(x1, touchX[0], x2) && BOUND(y1, touchY[0], y2))
          return true;
      if (touchN == 2 && ((BOUND(x1, touchX[0], x2) && BOUND(y1, touchY[0], y2)) ||
                          (BOUND(x1, touchX[1], x2) && BOUND(y1, touchY[1], y2))))
          return true;
    }

    return false;
  }

#endif