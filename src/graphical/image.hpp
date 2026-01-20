/*
image.hpp
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

#ifndef __IMAGE_HPP__
#define __IMAGE_HPP__

#include <cstdio>
#include <string>

#include "defines.hpp"
#include "adafruit_gfx.hpp"

class Image : virtual public Adafruit_GFX
{
  private:
    int16_t e_ink_width, e_ink_height;

    struct bitmapHeader {
        uint16_t signature;
        uint32_t fileSize;
        uint32_t startRAW;
        uint32_t dibHeaderSize;
        uint32_t width;
        uint32_t height;
        uint16_t color;
        uint32_t compression;
    };

  public:

    typedef enum
    {
        BMP,
        JPG,
        PNG
    } Format;

    typedef enum
    {
        Center,
        TopLeft,
        BottomLeft,
        TopRight,
        BottomRight
    } Position;
	
    Image(int16_t w, int16_t h);

    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

    virtual void        selectDisplayMode(DisplayMode _mode) = 0;
    virtual DisplayMode getDisplayMode() = 0;
    virtual int16_t     width()  = 0;
    virtual int16_t     height() = 0;

    bool drawImage(const char *path, int x, int y, bool dither = 1, bool invert = 0);
    bool drawImage(const std::string path, int x, int y, bool dither = 1, bool invert = 0);
    bool drawImage(const uint8_t *buf, int x, int y, int16_t w, int16_t h, uint8_t c = BLACK, uint8_t bg = 0xFF);
    bool drawImage(const char *path, const Format& format, const int x, const int y, const bool dither = 1, const bool invert = 0);
    bool drawImage(const std::string path, const Format& format, const int x, const int y, const bool dither = 1, const bool invert = 0);
    bool drawImage(const char* path, const Format& format, const Position& position, const bool dither = 1, const bool invert = 0);	

    bool drawBitmapFromFile(const char *fileName, int x, int y, bool dither = 0, bool invert = 0);
    bool   drawJpegFromFile(const char *fileName, int x, int y, bool dither = 0, bool invert = 0);
    bool    drawPngFromFile(const char *fileName, int x, int y, bool dither = 0, bool invert = 0);

    bool drawBitmapFromFile(FILE *p, int x, int y, bool dither = 0, bool invert = 0);
    bool   drawJpegFromFile(FILE *p, int x, int y, bool dither = 0, bool invert = 0);
    bool    drawPngFromFile(FILE *p, int x, int y, bool dither = 0, bool invert = 0);

    bool  drawBitmapFromWeb(const char *url, int x, int y, bool dither = 0, bool invert = 0);
    bool    drawJpegFromWeb(const char *url, int x, int y, bool dither = 0, bool invert = 0);
    bool     drawPngFromWeb(const char *url, int x, int y, bool dither = 0, bool invert = 0);

    // Defined in Adafruit-GFX-Library, but should fit here
    // void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    // void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color,
    //                        uint16_t bg);

    void drawBitmap3Bit(int16_t _x, int16_t _y, const unsigned char *_p, int16_t _w, int16_t _h);

    bool drawBitmapFromBuffer(uint8_t *buf, int x, int y, bool dither, bool invert);

    bool drawJpegFromBuffer(uint8_t *buf, int32_t len, int x, int y, bool dither, bool invert);

    // Should be private, but needed in a png callback :(
    void           ditherSwap(int w);
    uint8_t ditherGetPixelBmp(uint8_t px, int i, int w, bool paletted);

    static inline int16_t rowSize(int16_t w, int8_t c) { return ((c * w + 31) >> 5) << 2; }

    static inline uint32_t read32(uint8_t * c) { return (uint32_t)(c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24)); }
    static inline uint16_t read16(uint8_t * c) { return (uint16_t)(c[0] | (c[1] << 8));                               }

    static inline uint8_t rgb3Bit(uint8_t r, uint8_t g, uint8_t b) { return (54UL * (r) + 183UL * (g) + 19UL * (b)) >> 13; }
    static inline uint8_t rgb8Bit(uint8_t r, uint8_t g, uint8_t b) { return (54UL * (r) + 183UL * (g) + 19UL * (b)) >>  8; }

    static inline uint8_t     red(uint16_t a) { return ((a & 0xf800) >> 11) << 3; }
    static inline uint8_t   green(uint16_t a) { return ((a & 0x07e0) >>  5) << 2; }
    static inline uint8_t    blue(uint16_t a) { return  (a & 0x001f)        << 3; }
    
  private:
    virtual void     startWrite(void) = 0;
    virtual void     writePixel(int16_t  x, int16_t  y, uint16_t color) = 0;
    virtual void  writeFillRect(int16_t  x, int16_t  y, int16_t  w, int16_t  h, uint16_t color) = 0;
    virtual void writeFastVLine(int16_t  x, int16_t  y, int16_t  h, uint16_t color) = 0;
    virtual void writeFastHLine(int16_t  x, int16_t  y, int16_t  w, uint16_t color) = 0;
    virtual void      writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t  y1, uint16_t color) = 0;
    virtual void       endWrite(void) = 0;

    static bool   drawJpegChunk(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap, bool dither, bool invert);

    // uint8_t pixelBuffer[e_ink_width * 4 + 5];
    // uint8_t ditherBuffer[2][e_ink_width + 20];

    uint8_t * pixelBuffer;  //[e_ink_width * 4 + 5];
    uint8_t * ditherBuffer; //[2 * e_ink_width + 20];

    int16_t ditherBufferSize;
    int16_t pixelBufferSize;

    uint8_t jpegDitherBuffer[18][18];
    int16_t blockW = 0, blockH = 0;
    int16_t lastY = -1;

    uint8_t ditherPalette[256]; // 8 bit colors
    uint8_t palette[128];       // 2 3 bit colors per byte, _###_###

    bool legalBmp(bitmapHeader *bmpHeader);

    uint8_t ditherGetPixelJpeg(uint8_t px, int i, int j, int x, int y, int w, int h);
    void ditherSwapBlockJpeg(int x);

    void readBmpHeader(uint8_t *buf, bitmapHeader *_h);
    void readBmpHeaderFromFile(FILE *_f, bitmapHeader *_h);

    inline void displayBmpLine(int16_t x, int16_t y, bitmapHeader *bmpHeader, bool dither, bool invert);

    void getPointsForPosition(const Position& position, const uint16_t imageWidth, const uint16_t imageHeight, 
		const uint16_t screenWidth, const uint16_t screenHeight, uint16_t *posX, uint16_t *posY);

    bool drawJpegFromWebAtPosition(const char* url, const Position& position, const bool dither = 0, const bool invert = 0);

    // FUTURE COMPATIBILITY FUNCTIONS; DO NOT USE!

    void drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);

    void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t * bitmap, int16_t w, int16_t h);
    void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t * bitmap, uint8_t * mask, int16_t w, int16_t h);

    void drawRGBBitmap(int16_t x, int16_t y, uint16_t * bitmap, int16_t w, int16_t h);
    void drawRGBBitmap(int16_t x, int16_t y, uint16_t * bitmap, uint8_t * mask, int16_t w, int16_t h);
    
    // -------------------------------------------
};

#endif
