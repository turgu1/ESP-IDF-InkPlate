/*
eink_6.cpp
Inkplate 6 ESP-IDF

Modified by Guy Turcotte 
July 20, 2024

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

#if INKPLATE_6

#define __EINK6__ 1
#include "eink_6.hpp"
#include "esp_log.h"

#include "wire.hpp"
#include "esp.hpp"

#include <iostream>

const uint8_t EInk6::WAVEFORM_3BIT[8][8] = {
  {0, 1, 1, 0, 0, 1, 1, 0}, {0, 1, 2, 1, 1, 2, 1, 0}, 
  {1, 1, 1, 2, 2, 1, 0, 0}, {0, 0, 0, 1, 1, 1, 2, 0},
  {2, 1, 1, 1, 2, 1, 2, 0}, {2, 2, 1, 1, 2, 1, 2, 0}, 
  {1, 1, 1, 2, 1, 2, 2, 0}, {0, 0, 0, 0, 0, 0, 2, 0}};

const uint8_t EInk6::LUT2[16] = {
  0xAA, 0xA9, 0xA6, 0xA5, 0x9A, 0x99, 0x96, 0x95,
  0x6A, 0x69, 0x66, 0x65, 0x5A, 0x59, 0x56, 0x55 };

const uint8_t EInk6::LUTW[16] = {
  0xFF, 0xFE, 0xFB, 0xFA, 0xEF, 0xEE, 0xEB, 0xEA,
  0xBF, 0xBE, 0xBB, 0xBA, 0xAF, 0xAE, 0xAB, 0xAA };

const uint8_t EInk6::LUTB[16] = {
  0xFF, 0xFD, 0xF7, 0xF5, 0xDF, 0xDD, 0xD7, 0xD5,
  0x7F, 0x7D, 0x77, 0x75, 0x5F, 0x5D, 0x57, 0x55 };

bool 
EInk6::setup()
{
  if (initialized) return true;

  ESP_LOGD(TAG, "Initializing...");

  wire.setup();
  
  if (!io_expander_int.setup()) {
    ESP_LOGE(TAG, "Initialization not completed (MCP Issue).");
    return false;
  }
  else {
    ESP_LOGD(TAG, "MCP initialized.");
  }

  Wire::enter();
  
  io_expander_int.set_direction(VCOM,         IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(PWRUP,        IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(WAKEUP,       IOExpander::PinMode::OUTPUT); 
  io_expander_int.set_direction(GPIO0_ENABLE, IOExpander::PinMode::OUTPUT);
  io_expander_int.digital_write(GPIO0_ENABLE, IOExpander::SignalLevel::HIGH);

  wakeup_set(); 
 
  //ESP_LOGD(TAG, "Power Mgr Init..."); fflush(stdout);

  ESP::delay_microseconds(1800);
  wire.begin_transmission(PWRMGR_ADDRESS);
  wire.write(0x09);
  wire.write(0b00011011); // Power up seq.
  wire.write(0b00000000); // Power up delay (3mS per rail)
  wire.write(0b00011011); // Power down seq.
  wire.write(0b00000000); // Power down delay (6mS per rail)
  wire.end_transmission();
  ESP::delay(1);

  //ESP_LOGD(TAG, "Power init completed");

  wakeup_clear();

  // CONTROL PINS
  gpio_set_direction(GPIO_NUM_0,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_2,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);

  io_expander_int.set_direction(OE,      IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(GMOD,    IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(SPV,     IOExpander::PinMode::OUTPUT);

  // DATA PINS
  gpio_set_direction(GPIO_NUM_4,  GPIO_MODE_OUTPUT); // D0
  gpio_set_direction(GPIO_NUM_5,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_25, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT); // D7

  d_memory_new = new_frame_buffer_1bit();
  p_buffer     = (uint8_t *)  ESP::ps_malloc(BITMAP_SIZE_1BIT * 2);

  GLUT  = (uint32_t *)malloc(256 * 8 * sizeof(uint32_t));
  GLUT2 = (uint32_t *)malloc(256 * 8 * sizeof(uint32_t));

  ESP_LOGD(TAG, "Memory allocation for frame/bitmap buffers.");
  ESP_LOGD(TAG, "d_memory_new: %08x p_buffer: %08x.", (unsigned int)d_memory_new, (unsigned int)p_buffer);

  Wire::leave();
  
  if ((d_memory_new == nullptr) || 
      (p_buffer     == nullptr) ||
      (GLUT         == nullptr) ||
      (GLUT2        == nullptr)) {
    return false;
  }


  d_memory_new->clear();
  memset(p_buffer, 0, BITMAP_SIZE_1BIT * 2);

  for (int i = 0; i < 8; ++i) {
    for (uint32_t j = 0; j < 256; ++j) {
      uint8_t z = (WAVEFORM_3BIT[j & 0x07][i] << 2) | (WAVEFORM_3BIT[(j >> 4) & 0x07][i]);
      GLUT[i * 256 + j] = PIN_LUT[z];
      z = ((WAVEFORM_3BIT[j & 0x07][i] << 2) | (WAVEFORM_3BIT[(j >> 4) & 0x07][i])) << 4;
      GLUT2[i * 256 + j] = PIN_LUT[z];
    }
  }

  initialized = true;

  return true;
}

void
EInk6::update(FrameBuffer1Bit & frame_buffer)
{
  ESP_LOGD(TAG, "1bit Update...");
 
  const uint8_t * ptr;
  uint32_t        send;
  uint8_t         dram;

  Wire::enter();

  if (!turn_on()) {
    Wire::leave();
    return;
  }

  clean(PixelState::WHITE,      1);
  clean(PixelState::BLACK,     21);
  clean(PixelState::DISCHARGE,  1);
  clean(PixelState::WHITE,     12);
  clean(PixelState::DISCHARGE,  1);
  clean(PixelState::BLACK,     21);
  clean(PixelState::DISCHARGE,  1);
  clean(PixelState::WHITE,     12);

  uint8_t * data = frame_buffer.get_data();

  for (int8_t k = 0; k < 4; k++) {
    ptr = &data[BITMAP_SIZE_1BIT - 1];
    vscan_start();

    for (uint16_t i = 0; i < HEIGHT; i++) {
      dram = *ptr--;
      send = PIN_LUT[LUTB[dram >> 4]];
      hscan_start(send);
      send = PIN_LUT[LUTB[dram & 0x0F]];
      GPIO.out_w1ts = CL | send;
      GPIO.out_w1tc = CL | DATA;

      for (uint16_t j = 0; j < LINE_SIZE_1BIT - 1; j++) {
        dram = *ptr--;
        send = PIN_LUT[LUTB[dram >> 4]];
        GPIO.out_w1ts = CL | send;
        GPIO.out_w1tc = CL | DATA;
        send = PIN_LUT[LUTB[dram & 0x0F]];
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
    send = PIN_LUT[LUT2[dram & 0x0F]];
    GPIO.out_w1ts = CL | send;
    GPIO.out_w1tc = CL | DATA;
    
    for (uint16_t j = 0; j < LINE_SIZE_1BIT - 1; j++) {
      dram = *ptr--;
      send = PIN_LUT[LUT2[dram >> 4]];
      GPIO.out_w1ts = CL | send;
      GPIO.out_w1tc = CL | DATA;
      send = PIN_LUT[LUT2[dram & 0x0F]];
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

void
EInk6::update(FrameBuffer3Bit & frame_buffer)
{
  ESP_LOGD(TAG, "3bit Update...");

  Wire::enter();
  if (!turn_on()) {
    Wire::leave();
    return;
  }

  clean(PixelState::WHITE,      1);
  clean(PixelState::BLACK,     21);
  clean(PixelState::DISCHARGE,  1);
  clean(PixelState::WHITE,     12);
  clean(PixelState::DISCHARGE,  1);
  clean(PixelState::BLACK,     21);
  clean(PixelState::DISCHARGE,  1);
  clean(PixelState::WHITE,     12);

  uint8_t * data = frame_buffer.get_data();

  for (int k = 0, kk = 0; k < 8; k++, kk += 256) {

    const uint8_t * dp = &data[BITMAP_SIZE_3BIT - 2];

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

void
EInk6::partial_update(FrameBuffer1Bit & frame_buffer, bool force)
{
  if (!is_partial_allowed() && !force) {
    update(frame_buffer);
    return;
  }

  Wire::enter();

  ESP_LOGD(TAG, "Partial update...");

  uint8_t * idata = frame_buffer.get_data();
  uint8_t * odata = d_memory_new->get_data();

  uint32_t n   = BITMAP_SIZE_1BIT * 2 - 1;
  uint16_t pos = BITMAP_SIZE_1BIT - 1;

  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < LINE_SIZE_1BIT; j++) {
      uint8_t diffw =  odata[pos] & ~idata[pos];
      uint8_t diffb = ~odata[pos] &  idata[pos];
      pos--;
      p_buffer[n--] = LUTW[diffw >>   4] & (LUTB[diffb >>   4]);
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
        send = PIN_LUT[p_buffer[n--]];
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
  clean(PixelState::SKIP,      1);
  
  vscan_start();
  turn_off();

  Wire::leave();
  memcpy(d_memory_new->get_data(), frame_buffer.get_data(), BITMAP_SIZE_1BIT);
}

void
EInk6::clean(PixelState pixel_state, uint8_t repeat_count)
{
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