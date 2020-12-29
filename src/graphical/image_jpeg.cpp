/*
image_jpeg.cpp
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

#include <cstdio>
#include <sys/stat.h>

#include "image.hpp"

#include "tjpg_decoder.hpp"
#include "network_client.hpp"
#include "inkplate_platform.hpp"

extern Image *_imagePtrJpeg;

bool Image::drawJpegFromFile(const char * fileName, int x, int y, bool dither, bool invert)
{
    FILE * dat = fopen(fileName, "r");
    if (dat)
        return drawJpegFromFile(dat, x, y, dither, invert);
    return 0;
}

bool Image::drawJpegFromFile(FILE * p, int x, int y, bool dither, bool invert)
{
    uint8_t ret = 0;

    blockW = -1;
    blockH = -1;
    lastY = -1;
    memset(ditherBuffer, 0, ditherBufferSize);

    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(drawJpegChunk);

    struct stat stat_buf;
    fstat(fileno(p), &stat_buf);
    uint32_t total = stat_buf.st_size;

    uint8_t *buff = (uint8_t *)malloc(total);

    if (buff == NULL) return 0;

    if (fread(buff, total, 1, p) != 1) return 0;

    fclose(p);

    if (TJpgDec.drawJpg(x, y, buff, total, dither, invert) == 0) ret = 1;

    free(buff);

    return ret;
}

bool Image::drawJpegFromWeb(const char *url, int x, int y, bool dither, bool invert)
{
    bool ret = 0;

    int32_t defaultLen = e_ink_width * e_ink_height * 4;
    uint8_t *buff = network_client.downloadFile(url, &defaultLen);

    ret = drawJpegFromBuffer(buff, defaultLen, x, y, dither, invert);
    free(buff);

    return ret;
}

bool Image::drawJpegFromWebAtPosition(const char *url, const Position& position, const bool dither, const bool invert)
{
    bool ret = 0;

    int32_t defaultLen = 800 * 600 * 4;
    uint8_t *buff = network_client.downloadFile(url, &defaultLen);

    uint16_t w = 0;
    uint16_t h = 0;
    TJpgDec.setJpgScale(1);
    JRESULT r = TJpgDec.getJpgSize(&w, &h, buff, defaultLen);
    if(r != JDR_OK) {
        free(buff);
        return false;
    }

    uint16_t posX, posY;
    getPointsForPosition(position, w, h, 800, 600, &posX, &posY);

    ret = drawJpegFromBuffer(buff, defaultLen, posX, posY, dither, invert);
    free(buff);

    return ret;
}

bool Image::drawJpegFromBuffer(uint8_t *buff, int32_t len, int x, int y, bool dither, bool invert)
{
    bool ret = 0;

    blockW = -1;
    blockH = -1;
    lastY = -1;

    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(drawJpegChunk);

    int err = TJpgDec.drawJpg(x, y, buff, len, dither, invert);
    if (err == 0)
        ret = 1;

    return ret;
};

bool Image::drawJpegChunk(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap, bool dither, bool invert)
{
    if (!_imagePtrJpeg)
        return 0;

    if (dither && y != _imagePtrJpeg->lastY)
    {
        _imagePtrJpeg->ditherSwap(e_ink.get_width());
        _imagePtrJpeg->lastY = y;
    }

    _imagePtrJpeg->startWrite();
    for (int j = 0; j < h; ++j)
    {
        for (int i = 0; i < w; ++i)
        {
            uint16_t rgb = bitmap[j * w + i];
            uint8_t val;
            if (dither)
                val = _imagePtrJpeg->ditherGetPixelJpeg(rgb8Bit(red(rgb), green(rgb), blue(rgb)), i, j, x, y, w, h);
            else
                val = rgb3Bit(red(rgb), green(rgb), blue(rgb));
            if (invert)
                val = 7 - val;
            if (_imagePtrJpeg->getDisplayMode() == DisplayMode::INKPLATE_1BIT)
                val = (~val >> 2) & 1;
            _imagePtrJpeg->writePixel(x + i, y + j, val);
        }
    }
    if (dither)
        _imagePtrJpeg->ditherSwapBlockJpeg(x);
    _imagePtrJpeg->endWrite();

    return 1;
}
