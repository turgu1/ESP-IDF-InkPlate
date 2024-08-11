/*
image_bmp.cpp
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

#include "image.hpp"
#include "network_client.hpp"

#include <cstdio>

bool Image::legalBmp(bitmapHeader *bmpHeader)
{
    return bmpHeader->signature == 0x4D42 && bmpHeader->compression == 0 &&
           (bmpHeader->color == 1 || bmpHeader->color == 4 || bmpHeader->color == 8 || bmpHeader->color == 16 ||
            bmpHeader->color == 24 || bmpHeader->color == 32);
}

void Image::readBmpHeaderFromFile(FILE * f, bitmapHeader * h)
{
    uint8_t header[55];

    rewind(f);
    fread(header, 55, 1, f);

    uint16_t color       = read16(header + 28);
    uint32_t totalColors = read32(header + 46);

    if (color <= 8)
    {
        if (!totalColors)
            totalColors = (1ULL << color);

        uint8_t * buff = new uint8_t[totalColors * 4 + 100];

        rewind(f);
        fread(buff, totalColors * 4 + 100, 1, f);

        readBmpHeader(buff, h);
        delete[] buff;
    }
    else
    {
        readBmpHeader(header, h);
    }
}

void Image::readBmpHeader(uint8_t *buf, bitmapHeader *_h)
{
    _h->signature     = read16(buf +  0);
    _h->fileSize      = read32(buf +  2);
    _h->startRAW      = read32(buf + 10);
    _h->dibHeaderSize = read32(buf + 14);
    _h->width         = read32(buf + 18);
    _h->height        = read32(buf + 22);
    _h->color         = read16(buf + 28);
    _h->compression   = read32(buf + 30);

    uint32_t totalColors = read32(buf + 46);

    uint8_t paletteRGB[1024];

    if (_h->color <= 8)
    {
        if (!totalColors)
            totalColors = (1ULL << _h->color);

        memcpy(paletteRGB, buf + 53, totalColors * 4);
        memset(palette, 0, sizeof palette);

        for (int i = 0; i < totalColors; ++i)
        {
            uint32_t c = read32(paletteRGB + (i << 2));

            uint8_t r = (c & 0xFF000000) >> 24;
            uint8_t g = (c & 0x00FF0000) >> 16;
            uint8_t b = (c & 0x0000FF00) >> 8;

            palette[i >> 1] |= rgb3Bit(r, g, b) << (i & 1 ? 0 : 4);
            ditherPalette[i] = rgb8Bit(r, g, b);
        }
    }
};

bool Image::drawBitmapFromFile(const char *fileName, int x, int y, bool dither, bool invert)
{
    FILE * dat = fopen(fileName, "r");
    if (dat)
        return drawBitmapFromFile(dat, x, y, dither, invert);
    else
        return 0;
}

bool Image::drawBitmapFromFile(FILE * p, int x, int y, bool dither, bool invert)
{
    bitmapHeader bmpHeader;

    readBmpHeaderFromFile(p, &bmpHeader);

    if (!legalBmp(&bmpHeader))
        return 0;

    int16_t w = bmpHeader.width, h = bmpHeader.height;
    int8_t c = bmpHeader.color;

    fseek(p, bmpHeader.startRAW, SEEK_SET);
    if (dither)
        memset(ditherBuffer, 0, ditherBufferSize);
        
    for (int i = 0; i < h; ++i)
    {
        int16_t n = rowSize(w, c);
        fread(pixelBuffer, n, 1, p);
        displayBmpLine(x, y + bmpHeader.height - i - 1, &bmpHeader, dither, invert);
    }
    return 1;
}

bool Image::drawBitmapFromWeb(const char *url, int x, int y, bool dither, bool invert)
{
    bool ret = 0;
    int32_t defaultLen = e_ink_width * e_ink_height * 4 + 150;
    uint8_t *buf = network_client.downloadFile(url, &defaultLen);

    ret = drawBitmapFromBuffer(buf, x, y, dither, invert);
    free(buf);

    return ret;
}

bool Image::drawBitmapFromBuffer(uint8_t *buf, int x, int y, bool dither, bool invert)
{
    bitmapHeader bmpHeader;

    readBmpHeader(buf, &bmpHeader);

    if (!legalBmp(&bmpHeader))
        return 0;

    if (dither)
        memset(ditherBuffer, 0, ditherBufferSize);

    uint8_t *bufferPtr = buf + bmpHeader.startRAW;
    for (int i = 0; i < bmpHeader.height; ++i)
    {
        memcpy(pixelBuffer, bufferPtr, rowSize(bmpHeader.width, bmpHeader.color));
        displayBmpLine(x, y + bmpHeader.height - i - 1, &bmpHeader, dither, invert);
        bufferPtr += rowSize(bmpHeader.width, bmpHeader.color);
    }

    return 1;
}

void Image::displayBmpLine(int16_t x, int16_t y, bitmapHeader *bmpHeader, bool dither, bool invert)
{
    int16_t w = bmpHeader->width;
    int8_t c = bmpHeader->color;

    startWrite();
    for (int j = 0; j < w; ++j)
    {
        switch (c)
        {
        case 1:
            writePixel(x + j, y, (invert ^ (palette[0] > palette[1])) ^ !!(pixelBuffer[j >> 3] & (1 << (7 - (j & 7)))));
            break;
        // as for 2 bit, literally cannot find an example online or in PS, so skipped
        case 4: {
            uint8_t px = (pixelBuffer[j >> 1] & (j & 1 ? 0x0F : 0xF0)) >> (j & 1 ? 0 : 4);
            uint8_t val;

            if (dither)
                val = ditherGetPixelBmp(px, j, w, 1);
            else
                val = (palette[px >> 1] & (px & 1 ? 0x0F : 0xF0)) >> (px & 1 ? 0 : 4);
            if (invert)
                val = 7 - val;
            if (getDisplayMode() == DisplayMode::INKPLATE_1BIT)
                val = (~val >> 2) & 1;

            writePixel(x + j, y, val);
            break;
        }
        case 8: {
            uint8_t px = pixelBuffer[j];
            uint8_t val;

            if (dither)
                val = ditherGetPixelBmp(px, j, w, 1);
            else
                val = (palette[px >> 1] & (px & 1 ? 0x0F : 0xF0)) >> (px & 1 ? 0 : 4);
            if (invert)
                val = 7 - val;
            if (getDisplayMode() == DisplayMode::INKPLATE_1BIT)
                val = (~val >> 2) & 1;

            writePixel(x + j, y, val);
            break;
        }
        case 16: {
            uint16_t px = ((uint16_t)pixelBuffer[(j << 1) | 1] << 8) | pixelBuffer[(j << 1)];

            uint8_t r = (px & 0x7C00) >> 7;
            uint8_t g = (px & 0x3E0) >> 2;
            uint8_t b = (px & 0x1F) << 3;

            uint8_t val;

            if (dither)
                val = ditherGetPixelBmp(rgb8Bit(r, g, b), j, w, 0);
            else
                val = rgb3Bit(r, g, b);
            if (invert)
                val = 7 - val;
            if (getDisplayMode() == DisplayMode::INKPLATE_1BIT)
                val = (~val >> 2) & 1;

            writePixel(x + j, y, val);
            break;
        }
        case 24: {
            uint8_t r = pixelBuffer[j * 3];
            uint8_t g = pixelBuffer[j * 3 + 1];
            uint8_t b = pixelBuffer[j * 3 + 2];

            uint8_t val;

            if (dither)
                val = ditherGetPixelBmp(rgb8Bit(r, g, b), j, w, 0);
            else
                val = rgb3Bit(r, g, b);
            if (invert)
                val = 7 - val;
            if (getDisplayMode() == DisplayMode::INKPLATE_1BIT)
                val = (~val >> 2) & 1;

            writePixel(x + j, y, val);
            break;
        }
        case 32:
            uint8_t r = pixelBuffer[j * 4];
            uint8_t g = pixelBuffer[j * 4 + 1];
            uint8_t b = pixelBuffer[j * 4 + 2];

            uint8_t val;

            if (dither)
                val = ditherGetPixelBmp(rgb8Bit(r, g, b), j, w, 0);
            else
                val = rgb3Bit(r, g, b);
            if (invert)
                val = 7 - val;
            if (getDisplayMode() == DisplayMode::INKPLATE_1BIT)
                val = (~val >> 2) & 1;

            writePixel(x + j, y, val);
            break;
        }
    }

    ditherSwap(w);
    endWrite();
}
