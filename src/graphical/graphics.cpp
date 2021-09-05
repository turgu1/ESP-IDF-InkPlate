/*
graphics.cpp
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

#include "graphics.hpp"
#include "inkplate_platform.hpp"
#include "esp_log.h"

#include <algorithm>

Graphics::Graphics(int16_t w, int16_t h) : 
  Adafruit_GFX(w, h), Shapes(w, h), Image(w, h) 
{
  _partial     = e_ink.new_frame_buffer_1bit();
  DMemory4Bit  = e_ink.new_frame_buffer_3bit();

  if ((_partial == nullptr) || (DMemory4Bit == nullptr)) {
    ESP_LOGE(TAG, "Unable to allocate PSRAM memory for buffers.");
  }
  else {
    _partial->clear();
    DMemory4Bit->clear();
  }
};

void Graphics::setRotation(uint8_t x)
{
    rotation = (x & 3);
    switch (rotation)
    {
    case 0:
    case 2:
        _width = e_ink.get_width();
        _height = e_ink.get_height();
        break;
    case 1:
    case 3:
        _width = e_ink.get_height();
        _height = e_ink.get_width();
        break;
    }
}

uint8_t Graphics::getRotation()
{
    return rotation;
}

void Graphics::drawPixel(int16_t x0, int16_t y0, uint16_t color)
{
    writePixel(x0, y0, color); // Specified in boards folder
}

void Graphics::startWrite()
{
}

void Graphics::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j)
            writePixel(x + j, y + i, color);
}

void Graphics::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    for (int i = 0; i < h; ++i)
        writePixel(x, y + i, color);
}

void Graphics::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    for (int j = 0; j < w; ++j)
        writePixel(x + j, y, color);
}

void Graphics::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx >> 1;
    int16_t ystep;

    if (y0 < y1)
        ystep = 1;
    else
        ystep = -1;

    for (; x0 <= x1; x0++)
    {
        if (steep)
            writePixel(y0, x0, color);
        else
            writePixel(x0, y0, color);
        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

void Graphics::endWrite()
{
}

void Graphics::selectDisplayMode(DisplayMode mode)
{
    if (mode != display_mode)
    {
        display_mode = mode;

        if (display_mode == DisplayMode::INKPLATE_1BIT)
          _partial->clear();
        else
          DMemory4Bit->clear();
    }
}

void Graphics::clearDisplay()
{
  if (display_mode == DisplayMode::INKPLATE_1BIT)
    _partial->clear();
  else
    DMemory4Bit->clear();
}

void Graphics::display()
{
  if (display_mode == DisplayMode::INKPLATE_1BIT) {
    ESP_LOGD(TAG, "Update 1Bit frame buffer");
    e_ink.update(*_partial);
  }
  else {
    ESP_LOGD(TAG, "Update 3Bit frame buffer");
    e_ink.update(*DMemory4Bit);
  }
}

void Graphics::preloadScreen()
{
  if (display_mode == DisplayMode::INKPLATE_1BIT) {
    e_ink.preload_screen(*_partial);
  }
}

void Graphics::partialUpdate(bool _forced)
{
  if (display_mode == DisplayMode::INKPLATE_1BIT) {
    e_ink.partial_update(*_partial, _forced);
  }
}

int16_t Graphics::width()
{
    return _width;
};

int16_t Graphics::height()
{
    return _height;
};

void Graphics::writePixel(int16_t x0, int16_t y0, uint16_t color)
{
    if (x0 > width() - 1 || y0 > height() - 1 || x0 < 0 || y0 < 0)
        return;

    switch (rotation)
    {
    case 1:
        std::swap(x0, y0);
        x0 = height() - x0 - 1;
        break;
    case 2:
        x0 = width() - x0 - 1;
        y0 = height() - y0 - 1;
        break;
    case 3:
        std::swap(x0, y0);
        y0 = width() - y0 - 1;
        break;
    }

    if (getDisplayMode() == DisplayMode::INKPLATE_1BIT)
    {
        int x = x0 >> 3;
        int x_sub = x0 & 7;
        uint8_t * p = &_partial->get_data()[_partial->get_line_size() * y0 + x];
        *p = (~pixelMaskLUT[x_sub] & *p) | (color ? pixelMaskLUT[x_sub] : 0);
    }
    else
    {
        color &= 7;
        int x = x0 >> 1;
        int x_sub = x0 & 1;
        uint8_t * p = &DMemory4Bit->get_data()[DMemory4Bit->get_line_size() * y0 + x];
        *p = (pixelMaskGLUT[x_sub] & *p) | (x_sub ? color : color << 4);
    }
}
