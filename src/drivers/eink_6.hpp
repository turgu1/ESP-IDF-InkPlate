/*
eink_6.hpp
Inkplate 6 ESP-IDF

Modified by Guy Turcotte
July 20, 2024

from the Arduino Library:

David Zovko, Borna Biro, Denis Vajak, Zvonimir Haramustek @ e-radionica.com
September 24, 2020
https://github.com/e-radionicacom/Inkplate-6-Arduino-library

For support, please reach over forums: forum.e-radionica.com/en
For more info about the product, please check: www.inkplate.io

This code is released under the GNU Lesser General Public License v3.0:
https://www.gnu.org/licenses/lgpl-3.0.en.html Please review the LICENSE file included with this
example. If you have any questions about licensing, please contact techsupport@e-radionica.com
Distributed as-is; no warranty is given.
*/

#if INKPLATE_6

#pragma once

#include <cinttypes>
#include <cstring>

#include "driver/gpio.h"
#include "eink.hpp"
#include "non_copyable.hpp"

#if PCAL6416
#include "pcal6416.hpp"
#else
#include "mcp23017.hpp"
#endif

/**
 * @brief Low level e-Ink display
 *
 * This class implements the low level methods required to control
 * and access the e-ink display of the InkPlate-6 device.
 *
 * This is a singleton. It cannot be instanciated elsewhere. It is not
 * instanciated in the heap. This is reinforced by the C++ construction
 * below. It also cannot be copied through the NonCopyable derivation.
 */

class EInk6 : public EInk, NonCopyable {
public:
  static const uint16_t WIDTH  = 800; // In pixels
  static const uint16_t HEIGHT = 600; // In pixels

  EInk6(IOExpander &io_expander) : EInk(io_expander, WIDTH) {}

  static const uint16_t BITMAP_SIZE_1BIT = (WIDTH * HEIGHT) >> 3;           // In bytes
  static const uint32_t BITMAP_SIZE_3BIT = ((uint32_t)WIDTH * HEIGHT) >> 1; // In bytes
  static const uint16_t LINE_SIZE_1BIT   = WIDTH >> 3;                      // In bytes
  static const uint16_t LINE_SIZE_3BIT   = WIDTH >> 1;                      // In bytes

  inline int16_t get_width() { return WIDTH; }
  inline int16_t get_height() { return HEIGHT; }

  inline PanelState get_panel_state() { return panel_state; }
  inline bool is_initialized() { return initialized; }

  virtual inline FrameBuffer1Bit *new_frame_buffer_1bit() { return new FrameBuffer1BitX; }
  virtual inline FrameBuffer3Bit *new_frame_buffer_3bit() { return new FrameBuffer3BitX; }

  // All the following methods are protecting the I2C device trough
  // the Wire::enter() and Wire::leave() methods. These are implementing a
  // Mutex semaphore access control.
  //
  // If you ever add public methods, you *MUST* consider adding calls to Wire::enter()
  // and Wire::leave() and insure no deadlock will happen... or modifu the mutex to use
  // a recursive mutex.

  bool setup();

  void update(FrameBuffer1Bit &frame_buffer);
  void update(FrameBuffer3Bit &frame_buffer);

  void partial_update(FrameBuffer1Bit &frame_buffer, bool force = false);

private:
  static constexpr char const *TAG = "EInk6";

  class FrameBuffer1BitX : public FrameBuffer1Bit {
  private:
    uint8_t data[BITMAP_SIZE_1BIT];

  public:
    FrameBuffer1BitX() : FrameBuffer1Bit(WIDTH, HEIGHT, BITMAP_SIZE_1BIT) {}

    uint8_t *get_data() { return data; }
  };

  class FrameBuffer3BitX : public FrameBuffer3Bit {
  private:
    uint8_t data[BITMAP_SIZE_3BIT];

  public:
    FrameBuffer3BitX() : FrameBuffer3Bit(WIDTH, HEIGHT, BITMAP_SIZE_3BIT) {}

    uint8_t *get_data() { return data; }
  };

  void clean(PixelState pixel_state, uint8_t repeat_count);

  static const uint8_t WAVEFORM_3BIT[8][8];
  static const uint32_t WAVEFORM[50];
  static const uint8_t LUT2[16];
  static const uint8_t LUTW[16];
  static const uint8_t LUTB[16];
};

#endif
