/*
eink_10.hpp
Inkplate 10 ESP-IDF

Modified by Guy Turcotte 
December 23, 2020

from the Arduino Library:

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

#if defined(INKPLATE_10)

#pragma once

#include <cinttypes>
#include <cstring>

#include "non_copyable.hpp"
#include "driver/gpio.h"
#include "mcp23017.hpp"
#include "eink.hpp"

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

class EInk10 : public EInk, NonCopyable
{
  static const uint8_t INKPLATE10_WAVEFORM_DEFAULT = 0;
  static const uint8_t INKPLATE10_WAVEFORM1 = 20;
  static const uint8_t INKPLATE10_WAVEFORM2 = 21;
  static const uint8_t INKPLATE10_WAVEFORM3 = 22;

  public:
    EInk10(MCP23017 & mcp_i, MCP23017 & mcp_e) : EInk(mcp_i), mcp_ext(mcp_e)
      { }  // Private constructor

    static const uint16_t WIDTH  = 1200; // In pixels
    static const uint16_t HEIGHT =  825; // In pixels
    static const uint32_t BITMAP_SIZE_1BIT = (WIDTH * HEIGHT) >> 3;            // In bytes
    static const uint32_t BITMAP_SIZE_3BIT = ((uint32_t) WIDTH * HEIGHT) >> 1; // In bytes
    static const uint16_t LINE_SIZE_1BIT   = WIDTH >> 3;                       // In bytes
    static const uint16_t LINE_SIZE_3BIT   = WIDTH >> 1;                       // In bytes

    inline int16_t  get_width() { return WIDTH;  }
    inline int16_t get_height() { return HEIGHT; }

    inline PanelState get_panel_state() { return panel_state; }
    inline bool        is_initialized() { return initialized; }

    virtual inline FrameBuffer1Bit * new_frame_buffer_1bit() { return new FrameBuffer1BitX; }
    virtual inline FrameBuffer3Bit * new_frame_buffer_3bit() { return new FrameBuffer3BitX; }

    // All the following methods are protecting the I2C device trough
    // the Wire::enter() and Wire::leave() methods. These are implementing a
    // Mutex semaphore access control.
    //
    // If you ever add public methods, you *MUST* consider adding calls to Wire::enter()
    // and Wire::leave() and insure no deadlock will happen... or modifu the mutex to use
    // a recursive mutex.

    bool setup();

    void update(FrameBuffer1Bit & frame_buffer);
    void update(FrameBuffer3Bit & frame_buffer);

    void partial_update(FrameBuffer1Bit & frame_buffer, bool force = false);
    
  private:
    static constexpr char const * TAG = "EInk10";

    MCP23017 & mcp_ext;
    
    class FrameBuffer1BitX : public FrameBuffer1Bit {
      private:
        uint8_t data[BITMAP_SIZE_1BIT];
      public:
        FrameBuffer1BitX() : FrameBuffer1Bit(WIDTH, HEIGHT, BITMAP_SIZE_1BIT) {}
       
        uint8_t * get_data() { return data; }
    };

    class FrameBuffer3BitX : public FrameBuffer3Bit {
      private:
        uint8_t data[BITMAP_SIZE_3BIT];
      public:
        FrameBuffer3BitX() : FrameBuffer3Bit(WIDTH, HEIGHT, BITMAP_SIZE_3BIT) {}

        uint8_t * get_data() { return data; }
    };

    void    clean(PixelState pixel_state, uint8_t repeat_count);
    uint8_t calculate_checksum(struct waveformData * w);
    bool    get_waveform_from_EEPROM(struct waveformData * w);
    void    calculate_LUTs();

    struct waveformData {
        uint8_t header = 'W';
        uint8_t waveform_id;
        uint8_t waveform[8][9];
        uint8_t temp = 20;
        uint8_t checksum;
    } waveform_EEPROM;

    uint8_t               waveform_3bit[8][9];
    uint8_t               current_waveform_id;

    static const uint8_t  DEFAULT_WAVEFORM_3BIT[8][9]; 
    static const uint8_t  LUT2[16];
    static const uint8_t  LUTW[16];
    static const uint8_t  LUTB[16];
};

#endif
