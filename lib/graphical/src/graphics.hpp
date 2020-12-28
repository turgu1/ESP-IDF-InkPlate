/*
graphics.hpp
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

#ifndef __GRAPHICS_HPP__
#define __GRAPHICS_HPP__

#include "image.hpp"
#include "shapes.hpp"
#include "frame_buffer.hpp"

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

class Graphics : public Shapes, public Image
{
  private:
    static constexpr char const * TAG = "Graphics";

    DisplayMode display_mode;  

  public:

    Graphics(int16_t w, int16_t h);

    void           setRotation(uint8_t r);
    uint8_t        getRotation();
    void             drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void     selectDisplayMode(DisplayMode mode);
    void        setDisplayMode(DisplayMode mode) { display_mode = mode; }
    DisplayMode getDisplayMode() { return display_mode; }
        
    void          clearDisplay();
    void               display();
    void         preloadScreen();
    void         partialUpdate(bool _forced = false);

    int16_t  width() override;
    int16_t height() override;

    FrameBuffer1Bit *_partial;
    FrameBuffer3Bit * DMemory4Bit;

    const uint8_t  pixelMaskLUT[8] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
    const uint8_t pixelMaskGLUT[2] = {0xF, 0xF0};

    const uint8_t discharge[16] = {0xFF, 0xFC, 0xF3, 0xF0, 0xCF, 0xCC, 0xC3, 0xC0,
                                   0x3F, 0x3C, 0x33, 0x30, 0xF,  0xC,  0x3,  0x0};

    uint8_t _blockPartial = 1;

  private:
    void     startWrite(void) override;
    void     writePixel(int16_t  x, int16_t  y, uint16_t color) override;
    void  writeFillRect(int16_t  x, int16_t  y, int16_t  w,  int16_t  h, uint16_t color) override;
    void writeFastVLine(int16_t  x, int16_t  y, int16_t  h,  uint16_t color) override;
    void writeFastHLine(int16_t  x, int16_t  y, int16_t  w,  uint16_t color) override;
    void      writeLine(int16_t x0, int16_t y0, int16_t  x1, int16_t  y1, uint16_t color) override;
    void       endWrite(void) override;
};

#endif