/*
eink_6v2.cpp
Inkplate 6V2 ESP-IDF

Modified by Guy Turcotte
Mars 11, 2026

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

#if INKPLATE_6V2

#define __EINK6V2__ 1
#include <iostream>

#include "eink_6v2.hpp"
#include "esp.hpp"
#include "esp_log.h"
#include "wire.hpp"

const uint8_t EInk6V2::WAVEFORM_3BIT[8][9] = {
    {0, 0, 0, 0, 1, 1, 1, 1, 0}, {0, 0, 0, 1, 1, 1, 1, 0, 0}, {1, 1, 1, 1, 0, 2, 1, 0, 0},
    {1, 1, 1, 2, 2, 1, 1, 0, 0}, {1, 1, 1, 1, 2, 2, 1, 0, 0}, {0, 1, 1, 1, 2, 2, 1, 0, 0},
    {0, 0, 0, 0, 1, 1, 2, 0, 0}, {0, 0, 0, 0, 0, 0, 2, 0, 0}};

const uint8_t EInk6V2::LUT2[16] = {0xAA, 0xA9, 0xA6, 0xA5, 0x9A, 0x99, 0x96, 0x95,
                                   0x6A, 0x69, 0x66, 0x65, 0x5A, 0x59, 0x56, 0x55};

const uint8_t EInk6V2::LUTW[16] = {0xFF, 0xFE, 0xFB, 0xFA, 0xEF, 0xEE, 0xEB, 0xEA,
                                   0xBF, 0xBE, 0xBB, 0xBA, 0xAF, 0xAE, 0xAB, 0xAA};

const uint8_t EInk6V2::LUTB[16] = {0xFF, 0xFD, 0xF7, 0xF5, 0xDF, 0xDD, 0xD7, 0xD5,
                                   0x7F, 0x7D, 0x77, 0x75, 0x5F, 0x5D, 0x57, 0x55};

bool EInk6V2::setup() {
  if (initialized) return true;

  ESP_LOGD(TAG, "Initializing...");

  wire.setup();

  ESP_LOGD(TAG, "IO Expander Setup...");

  if (!io_expander_int.setup()) {
    ESP_LOGE(TAG, "Initialization not completed (PCAL Issue).");
    return false;
  } else {
    ESP_LOGD(TAG, "PCAL initialized.");
  }

  Wire::enter();

  io_expander_int.set_direction(VCOM, IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(PWRUP, IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(WAKEUP, IOExpander::PinMode::OUTPUT);

  io_expander_int.set_direction(GPIO0_ENABLE, IOExpander::PinMode::OUTPUT);
  io_expander_int.digital_write(GPIO0_ENABLE, IOExpander::SignalLevel::HIGH);

  wakeup_set();

  // ESP_LOGD(TAG, "Power Mgr Init..."); fflush(stdout);

  wire_device = new WireDevice(PWRMGR_ADDRESS);
  if ((wire_device == nullptr) || !wire_device->is_initialized()) {
    ESP_LOGE(TAG, "Setup error: %s", wire_device == nullptr ? "NULL Device!" : "Not initialized!");
    return false;
  }

  uint8_t pgm[] = {
      0x09,       // cmd
      0b00011011, // Power up seq.
      0b00000000, // Power up delay (3mS per rail)
      0b00011011, // Power down seq.
      0b00000000  // Power down delay (6mS per rail)
  };

  ESP::delay(1);

  wire_device->write(pgm, sizeof(pgm));

  ESP::delay_microseconds(1800);

  // ESP_LOGD(TAG, "Power init completed");

  wakeup_clear();

    // Unused pins of first I/O expander have to be set as outputs.
  io_expander_int.set_direction(IOExpander::Pin::IOPIN_11, IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(IOExpander::Pin::IOPIN_12, IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(IOExpander::Pin::IOPIN_13, IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(IOExpander::Pin::IOPIN_14, IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(IOExpander::Pin::IOPIN_15, IOExpander::PinMode::OUTPUT);

  io_expander_int.digital_write(IOExpander::Pin::IOPIN_11, IOExpander::SignalLevel::LOW);
  io_expander_int.digital_write(IOExpander::Pin::IOPIN_12, IOExpander::SignalLevel::LOW);
  io_expander_int.digital_write(IOExpander::Pin::IOPIN_13, IOExpander::SignalLevel::LOW);
  io_expander_int.digital_write(IOExpander::Pin::IOPIN_14, IOExpander::SignalLevel::LOW);
  io_expander_int.digital_write(IOExpander::Pin::IOPIN_15, IOExpander::SignalLevel::LOW);

  // CONTROL PINS
  gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);

  io_expander_int.set_direction(OE, IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(GMOD, IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(SPV, IOExpander::PinMode::OUTPUT);

  #if DMA_ENABLE
    if (i2s_comms.is_ready()) { // instanciated through the EInk constructor
      i2s_comms.init(5);
    } else {
      ESP_LOGE(TAG, "I2SComms is not ready!!!");
      return false;
    }
  #endif

  d_memory_new = new_frame_buffer_1bit();
  p_buffer     = (uint8_t *)ESP::ps_malloc(BITMAP_SIZE_1BIT * 2);

  GLUT  = (uint32_t *)malloc(256 * 9 * sizeof(uint32_t));
  GLUT2 = (uint32_t *)malloc(256 * 9 * sizeof(uint32_t));

  ESP_LOGI(TAG, "Memory allocation for frame/bitmap buffers.");
  ESP_LOGI(TAG, "d_memory_new: %08x p_buffer: %08x.", (unsigned int)d_memory_new,
           (unsigned int)p_buffer);

  Wire::leave();

  if ((d_memory_new == nullptr) || (p_buffer == nullptr) || (GLUT == nullptr) ||
      (GLUT2 == nullptr)) {
    return false;
  }

  d_memory_new->clear();
  memset(p_buffer, 0, BITMAP_SIZE_1BIT * 2);

  for (int j = 0; j < 9; j++) {
    for (uint32_t i = 0; i < 256; i++) {
      GLUT[(j << 8) + i] = (WAVEFORM_3BIT[i & 0x07][j] << 2) | (WAVEFORM_3BIT[(i >> 4) & 0x07][j]);
      GLUT2[(j << 8) + i] =
          ((WAVEFORM_3BIT[i & 0x07][j] << 2) | (WAVEFORM_3BIT[(i >> 4) & 0x07][j])) << 4;
    }
  }

  initialized = true;

  return true;
}

#if DMA_ENABLE
  void EInk6V2::update(FrameBuffer1Bit &frame_buffer) {
    ESP_LOGI(TAG, "1bit Update...");

    const uint8_t *ptr;

    Wire::enter();
    if (!turn_on()) {
      Wire::leave();
      return;
    }

    clean(PixelState::WHITE,      1);
    clean(PixelState::BLACK,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::WHITE,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::BLACK,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::WHITE,     18);
    clean(PixelState::DISCHARGE,  1);

    uint8_t *data = frame_buffer.get_data();

    volatile uint8_t *line_buffer = i2s_comms.get_line_buffer();

    ESP_LOGI(TAG, "part 1...");

    for (int k = 0; k < 5; k++) {

      ptr = &data[BITMAP_SIZE_1BIT - 1];

      vscan_start();

      for (int i = 0; i < HEIGHT; i++) {
        for (int n = 0; n < (WIDTH / 4); n += 4) {
          uint8_t dram1      = *ptr--;
          uint8_t dram2      = *ptr--;
          line_buffer[n    ] = LUTB[(dram2 >> 4) & 0x0F]; // i + 2;
          line_buffer[n + 1] = LUTB[ dram2 & 0x0F];       // i + 3;
          line_buffer[n + 2] = LUTB[(dram1 >> 4) & 0x0F]; // i;
          line_buffer[n + 3] = LUTB[ dram1 & 0x0F];       // i + 1;
        }

        i2s_comms.send_data();
        vscan_end();
      }

      ESP::delay_microseconds(230);
    }

    ESP_LOGI(TAG, "part 2...");

    for (int k = 0; k < 1; k++) {

      ptr = &data[BITMAP_SIZE_1BIT - 1];

      vscan_start();

      for (int i = 0; i < HEIGHT; i++) {

        for (int n = 0; n < (WIDTH / 4); n += 4) {
          uint8_t dram1      = *ptr--;
          uint8_t dram2      = *ptr--;
          line_buffer[n    ] = LUT2[(dram2 >> 4) & 0x0F]; // i + 2;
          line_buffer[n + 1] = LUT2[ dram2 & 0x0F];       // i + 3;
          line_buffer[n + 2] = LUT2[(dram1 >> 4) & 0x0F]; // i;
          line_buffer[n + 3] = LUT2[ dram1 & 0x0F];       // i + 1;
        }

        i2s_comms.send_data();
        vscan_end();
      }

      ESP::delay_microseconds(230);
    }

    ESP_LOGI(TAG, "part 3...");

    for (int k = 0; k < 1; k++) {

      vscan_start();

      for (int i = 0; i < HEIGHT; i++) {

        for (int n = 0; n < (WIDTH / 4); n += 4) {
          line_buffer[n    ] = 0; // i + 2;
          line_buffer[n + 1] = 0; // i + 3;
          line_buffer[n + 2] = 0; // i;
          line_buffer[n + 3] = 0; // i + 1;
        }

        i2s_comms.send_data();
        vscan_end();
      }

      ESP::delay_microseconds(230);
    }

    ESP_LOGI(TAG, "The End...");

    vscan_start();
    turn_off();

    Wire::leave();

    memcpy(d_memory_new->get_data(), frame_buffer.get_data(), BITMAP_SIZE_1BIT);

    allow_partial();
  }

  void EInk6V2::update(FrameBuffer3Bit &frame_buffer) {
    ESP_LOGD(TAG, "3bit Update...");

    Wire::enter();
    if (!turn_on()) {
      Wire::leave();
      return;
    }

    clean(PixelState::WHITE,      1);
    clean(PixelState::BLACK,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::WHITE,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::BLACK,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::WHITE,     18);
    clean(PixelState::DISCHARGE,  1);

    uint8_t *data = frame_buffer.get_data();

    volatile uint8_t *line_buffer = i2s_comms.get_line_buffer();

    for (int k = 0, kk = 0; k < 9; k++, kk += 256) {
      uint8_t *dp = &data[BITMAP_SIZE_3BIT] - 2;

      vscan_start();

      for (int i = 0; i < HEIGHT; i++) {

        for (int j = 0; j < (WIDTH / 4); j += 4) {
          line_buffer[j + 2] = (GLUT2[kk + dp[1]] | GLUT[kk + dp[0]]); dp -= 2;
          line_buffer[j + 3] = (GLUT2[kk + dp[1]] | GLUT[kk + dp[0]]); dp -= 2;
          line_buffer[j    ] = (GLUT2[kk + dp[1]] | GLUT[kk + dp[0]]); dp -= 2;
          line_buffer[j + 1] = (GLUT2[kk + dp[1]] | GLUT[kk + dp[0]]); dp -= 2;
        }

        i2s_comms.send_data();

        vscan_end();
      }

      ESP::delay_microseconds(230);
    }

    clean(PixelState::SKIP, 1);

    vscan_start();
    turn_off();

    Wire::leave();
    block_partial();
  }

  void EInk6V2::partial_update(FrameBuffer1Bit &frame_buffer, bool force) {
    if (!is_partial_allowed() && !force) {
      update(frame_buffer);
      return;
    }

    ESP_LOGD(TAG, "Partial update...");

    uint8_t *idata = frame_buffer.get_data();
    uint8_t *odata = d_memory_new->get_data();

    uint32_t n   = BITMAP_SIZE_1BIT * 2 - 1;
    uint16_t pos = BITMAP_SIZE_1BIT - 1;

    for (int i = 0; i < HEIGHT; i++) {
      for (int j = 0; j < LINE_SIZE_1BIT; j++) {
        uint8_t diffw =  odata[pos] & ~idata[pos];
        uint8_t diffb = ~odata[pos] &  idata[pos];
        pos--;
        p_buffer[n--] = LUTW[diffw >>   4] & (LUTB[diffb >> 4]);
        p_buffer[n--] = LUTW[diffw & 0x0F] & (LUTB[diffb & 0x0F]);
      }
    }

    Wire::enter();
    if (!turn_on()) {
      Wire::leave();
      return;
    }

    volatile uint8_t *line_buffer = i2s_comms.get_line_buffer();
    i2s_comms.init_lldesc();

    if (line_buffer == nullptr) return;

    for (int k = 0; k < 6; k++) {

      vscan_start();
      n = (BITMAP_SIZE_1BIT * 2) - 1;

      for (int i = 0; i < HEIGHT; i++) {

        for (int j = 0; j < (WIDTH / 4); j += 4) {
          line_buffer[j + 2] = p_buffer[n];
          line_buffer[j + 3] = p_buffer[n - 1];
          line_buffer[j    ] = p_buffer[n - 2];
          line_buffer[j + 1] = p_buffer[n - 3];
          n -= 4;
        }

        i2s_comms.send_data();

        vscan_end();
      }
    }

    clean(PixelState::DISCHARGE, 2);
    clean(PixelState::SKIP, 1);

    vscan_start();
    turn_off();

    Wire::leave();
    memcpy(d_memory_new->get_data(), frame_buffer.get_data(), BITMAP_SIZE_1BIT);
  }

  void EInk6V2::clean(PixelState pixel_state, uint8_t repeat_count) {

    if (!turn_on()) return;

    volatile uint8_t *line_buffer = i2s_comms.get_line_buffer();

    for (int i = 0; i < (WIDTH / 4); i++) {
      line_buffer[i] = static_cast<uint8_t>(pixel_state);
    }

    i2s_comms.init_lldesc();

    for (int k = 0; k < repeat_count; k++) {

      vscan_start();

      for (int i = 0; i < HEIGHT; i++) {
        i2s_comms.send_data();
        vscan_end();
      }

      ESP::delay_microseconds(230);
    }
  }

#else // DMA_ENABLE not defined, use GPIO bit-banging

  void EInk6V2::update(FrameBuffer1Bit &frame_buffer) {
    ESP_LOGD(TAG, "1bit Update...");

    const uint8_t *ptr;
    uint32_t send;
    uint8_t dram;

    Wire::enter();

    if (!turn_on()) {
      Wire::leave();
      return;
    }

    clean(PixelState::WHITE,      1);
    clean(PixelState::BLACK,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::WHITE,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::BLACK,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::WHITE,     18);
    clean(PixelState::DISCHARGE,  1);

    uint8_t *data = frame_buffer.get_data();

    for (int8_t k = 0; k < 5; k++) {
      ptr = &data[BITMAP_SIZE_1BIT - 1];
      vscan_start();

      for (uint16_t i = 0; i < HEIGHT; i++) {
        dram = *ptr--;
        send = PIN_LUT[LUTB[dram >> 4]];
        hscan_start(send);
        send          = PIN_LUT[LUTB[dram & 0x0F]];
        GPIO.out_w1ts = CL | send;
        GPIO.out_w1tc = CL | DATA;

        for (uint16_t j = 0; j < LINE_SIZE_1BIT - 1; j++) {
          dram          = *ptr--;
          send          = PIN_LUT[LUTB[dram >> 4]];
          GPIO.out_w1ts = CL | send;
          GPIO.out_w1tc = CL | DATA;
          send          = PIN_LUT[LUTB[dram & 0x0F]];
          GPIO.out_w1ts = CL | send;
          GPIO.out_w1tc = CL | DATA;
        }

        GPIO.out_w1ts = CL | send;
        GPIO.out_w1tc = CL | DATA;
        vscan_end();
      }
      ESP::delay_microseconds(230);
    }

    ptr = &data[BITMAP_SIZE_1BIT - 1];
    vscan_start();

    for (uint16_t i = 0; i < HEIGHT; i++) {
      dram = *ptr--;
      send = PIN_LUT[LUT2[dram >> 4]];
      hscan_start(send);
      send          = PIN_LUT[LUT2[dram & 0x0F]];
      GPIO.out_w1ts = CL | send;
      GPIO.out_w1tc = CL | DATA;

      for (uint16_t j = 0; j < LINE_SIZE_1BIT - 1; j++) {
        dram          = *ptr--;
        send          = PIN_LUT[LUT2[dram >> 4]];
        GPIO.out_w1ts = CL | send;
        GPIO.out_w1tc = CL | DATA;
        send          = PIN_LUT[LUT2[dram & 0x0F]];
        GPIO.out_w1ts = CL | send;
        GPIO.out_w1tc = CL | DATA;
      }

      GPIO.out_w1ts = CL | send;
      GPIO.out_w1tc = CL | DATA;
      vscan_end();
    }
    ESP::delay_microseconds(230);

    vscan_start();

    send = PIN_LUT[0];
    for (uint16_t i = 0; i < HEIGHT; i++) {
      hscan_start(send);
      GPIO.out_w1ts = CL | send;
      GPIO.out_w1tc = CL | DATA;

      for (int j = 0; j < LINE_SIZE_1BIT - 1; j++) {
        GPIO.out_w1ts = CL | send;
        GPIO.out_w1tc = CL | DATA;
        GPIO.out_w1ts = CL | send;
        GPIO.out_w1tc = CL | DATA;
      }

      GPIO.out_w1ts = CL | send;
      GPIO.out_w1tc = CL | DATA;
      vscan_end();
    }

    ESP::delay_microseconds(230);

    vscan_start();
    turn_off();

    Wire::leave();

    memcpy(d_memory_new->get_data(), frame_buffer.get_data(), BITMAP_SIZE_1BIT);
    allow_partial();
  }

  void EInk6V2::update(FrameBuffer3Bit &frame_buffer) {
    ESP_LOGD(TAG, "3bit Update...");

    Wire::enter();
    if (!turn_on()) {
      Wire::leave();
      return;
    }

    clean(PixelState::WHITE,      1);
    clean(PixelState::BLACK,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::WHITE,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::BLACK,     18);
    clean(PixelState::DISCHARGE,  1);
    clean(PixelState::WHITE,     18);
    clean(PixelState::DISCHARGE,  1);

    uint8_t *data = frame_buffer.get_data();

    for (int k = 0, kk = 0; k < 9; k++, kk += 256) {

      const uint8_t *dp = &data[BITMAP_SIZE_3BIT - 2];

      vscan_start();

      for (int i = 0; i < HEIGHT; i++) {

        hscan_start((GLUT2[kk + dp[1]] | GLUT[kk + dp[0]]));
        dp -= 2;

        GPIO.out_w1ts = CL | (GLUT2[kk + dp[1]] | GLUT[kk + dp[0]]);
        GPIO.out_w1tc = CL | DATA;
        dp -= 2;

        for (int j = 0; j < ((WIDTH / 8) - 1); j++) {
          GPIO.out_w1ts = CL | (GLUT2[kk + dp[1]] | GLUT[kk + dp[0]]);
          GPIO.out_w1tc = CL | DATA;
          dp -= 2;
          GPIO.out_w1ts = CL | (GLUT2[kk + dp[1]] | GLUT[kk + dp[0]]);
          GPIO.out_w1tc = CL | DATA;
          dp -= 2;
        }

        GPIO.out_w1ts = CL;
        GPIO.out_w1tc = CL | DATA;

        vscan_end();
      }

      ESP::delay_microseconds(230);
    }

    clean(PixelState::SKIP, 1);
    vscan_start();
    turn_off();

    Wire::leave();
    block_partial();
  }

  void EInk6V2::partial_update(FrameBuffer1Bit &frame_buffer, bool force) {
    if (!is_partial_allowed() && !force) {
      update(frame_buffer);
      return;
    }

    Wire::enter();

    ESP_LOGD(TAG, "Partial update...");

    uint8_t *idata = frame_buffer.get_data();
    uint8_t *odata = d_memory_new->get_data();

    uint32_t n   = BITMAP_SIZE_1BIT * 2 - 1;
    uint16_t pos = BITMAP_SIZE_1BIT - 1;

    for (int i = 0; i < HEIGHT; i++) {
      for (int j = 0; j < LINE_SIZE_1BIT; j++) {
        uint8_t diffw = odata[pos] & ~idata[pos];
        uint8_t diffb = ~odata[pos] & idata[pos];
        pos--;
        p_buffer[n--] = LUTW[diffw >> 4] & (LUTB[diffb >> 4]);
        p_buffer[n--] = LUTW[diffw & 0x0F] & (LUTB[diffb & 0x0F]);
      }
    }

    if (!turn_on()) {
      Wire::leave();
      return;
    }

    for (int k = 0; k < 5; k++) {
      vscan_start();
      n = BITMAP_SIZE_1BIT * 2 - 1;

      for (int i = 0; i < HEIGHT; i++) {
        uint32_t send = PIN_LUT[p_buffer[n--]];
        hscan_start(send);

        for (int j = 0; j < ((WIDTH / 4) - 1); j++) {
          send          = PIN_LUT[p_buffer[n--]];
          GPIO.out_w1ts = CL | send;
          GPIO.out_w1tc = CL | DATA;
        }

        GPIO.out_w1ts = CL | send;
        GPIO.out_w1tc = CL | DATA;
        vscan_end();
      }
      ESP::delay_microseconds(230);
    }

    clean(PixelState::DISCHARGE, 2);
    clean(PixelState::SKIP, 1);

    vscan_start();
    turn_off();

    Wire::leave();
    memcpy(d_memory_new->get_data(), frame_buffer.get_data(), BITMAP_SIZE_1BIT);
  }

  void EInk6V2::clean(PixelState pixel_state, uint8_t repeat_count) {

    if (!turn_on()) return;

    uint32_t send = PIN_LUT[static_cast<uint8_t>(pixel_state)];

    for (int8_t k = 0; k < repeat_count; k++) {

      vscan_start();

      for (uint16_t i = 0; i < HEIGHT; i++) {

        hscan_start(send);

        GPIO.out_w1ts = CL | send;
        GPIO.out_w1tc = CL;

        for (uint16_t j = 0; j < LINE_SIZE_1BIT - 1; j++) {
          GPIO.out_w1ts = CL;
          GPIO.out_w1tc = CL;
          GPIO.out_w1ts = CL;
          GPIO.out_w1tc = CL;
        }
        GPIO.out_w1ts = CL;
        GPIO.out_w1tc = CL;

        vscan_end();
      }

      ESP::delay_microseconds(230);
    }
  }

#endif

#endif