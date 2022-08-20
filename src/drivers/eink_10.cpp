/*
eink_10.cpp
Inkplate 10 ESP-IDF

Modified by Guy Turcotte 
December 27, 2020

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

#define __EINK10__ 1
#include "eink_10.hpp"
#include "esp_log.h"

#include "wire.hpp"
#include "mcp23017.hpp"
#include "esp.hpp"
#include "nvs_mgr.hpp"

#include <iostream>

const uint8_t EInk10::DEFAULT_WAVEFORM_3BIT[8][9] = {
  {0, 0, 0, 0, 0, 0, 0, 1, 0}, {0, 0, 0, 2, 2, 2, 1, 1, 0},
  {0, 0, 2, 1, 1, 2, 2, 1, 0}, {0, 1, 2, 2, 1, 2, 2, 1, 0},
  {0, 0, 2, 1, 2, 2, 2, 1, 0}, {0, 2, 2, 2, 2, 2, 2, 1, 0},
  {0, 0, 0, 0, 0, 2, 1, 2, 0}, {0, 0, 0, 2, 2, 2, 2, 2, 0}};

const uint8_t EInk10::LUT2[16] = {
  0xAA, 0xA9, 0xA6, 0xA5, 0x9A, 0x99, 0x96, 0x95,
  0x6A, 0x69, 0x66, 0x65, 0x5A, 0x59, 0x56, 0x55 };

const uint8_t EInk10::LUTW[16] = {
  0xFF, 0xFE, 0xFB, 0xFA, 0xEF, 0xEE, 0xEB, 0xEA,
  0xBF, 0xBE, 0xBB, 0xBA, 0xAF, 0xAE, 0xAB, 0xAA };

const uint8_t EInk10::LUTB[16] = {
  0xFF, 0xFD, 0xF7, 0xF5, 0xDF, 0xDD, 0xD7, 0xD5,
  0x7F, 0x7D, 0x77, 0x75, 0x5F, 0x5D, 0x57, 0x55 };

bool 
EInk10::setup()
{
  if (initialized) return true;

  ESP_LOGD(TAG, "Initializing...");

  wire.setup();
  
  if (!mcp_int.setup()) {
    ESP_LOGE(TAG, "Initialization not completed (Main MCP Issue).");
    return false;
  }
  else {
    ESP_LOGD(TAG, "MCP initialized.");
  }

  mcp_ext.setup();

  Wire::enter();
  
  if ( get_waveform_from_EEPROM(&waveform_EEPROM) && 
      (waveform_EEPROM.waveform_id >= INKPLATE10_WAVEFORM1) &&
      (waveform_EEPROM.waveform_id <= INKPLATE10_WAVEFORM3)    ) {
    memcpy(waveform_3bit, waveform_EEPROM.waveform, sizeof(waveform_3bit));
    current_waveform_id = waveform_EEPROM.waveform_id;
    ESP_LOGI(TAG, "Waveform %d loaded from EEPROM", current_waveform_id);
  }
  else {
    memcpy(waveform_3bit, DEFAULT_WAVEFORM_3BIT, sizeof(waveform_3bit));
    current_waveform_id = INKPLATE10_WAVEFORM_DEFAULT;
    ESP_LOGI(TAG, "Wavefrom load failed! Upload new waveform in EEPROM. Using default waveform.");
  }

  mcp_int.set_direction(VCOM,         MCP23017::PinMode::OUTPUT);
  mcp_int.set_direction(PWRUP,        MCP23017::PinMode::OUTPUT);
  mcp_int.set_direction(WAKEUP,       MCP23017::PinMode::OUTPUT); 
  mcp_int.set_direction(GPIO0_ENABLE, MCP23017::PinMode::OUTPUT);
  mcp_int.digital_write(GPIO0_ENABLE, MCP23017::SignalLevel::HIGH);

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

  // Set all pins of seconds I/O expander to outputs, low.
  // For some reason, it draw more current in deep sleep when pins are set as inputs...
  if (mcp_ext.is_present()) {
    for (int i = 0; i < 15; i++) {
      mcp_ext.set_direction((MCP23017::Pin) i, MCP23017::PinMode::OUTPUT);
      mcp_ext.digital_write((MCP23017::Pin) i, MCP23017::SignalLevel::LOW);
    }
  }

  // For same reason, unused pins of first I/O expander have to be also set as outputs, low.
  mcp_int.set_direction(MCP23017::Pin::IOPIN_13, MCP23017::PinMode::OUTPUT);
  mcp_int.set_direction(MCP23017::Pin::IOPIN_14, MCP23017::PinMode::OUTPUT);  
  mcp_int.set_direction(MCP23017::Pin::IOPIN_15, MCP23017::PinMode::OUTPUT);  
  mcp_int.digital_write(MCP23017::Pin::IOPIN_13, MCP23017::SignalLevel::LOW);
  mcp_int.digital_write(MCP23017::Pin::IOPIN_14, MCP23017::SignalLevel::LOW);
  mcp_int.digital_write(MCP23017::Pin::IOPIN_15, MCP23017::SignalLevel::LOW);

  // CONTROL PINS
  gpio_set_direction(GPIO_NUM_0,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_2,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);

  mcp_int.set_direction(OE,      MCP23017::PinMode::OUTPUT);
  mcp_int.set_direction(GMOD,    MCP23017::PinMode::OUTPUT);
  mcp_int.set_direction(SPV,     MCP23017::PinMode::OUTPUT);

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
  p_buffer     = (uint8_t *) malloc(BITMAP_SIZE_1BIT * 2);

  GLUT  = (uint32_t *) malloc(256 * 9 * sizeof(uint32_t));
  GLUT2 = (uint32_t *) malloc(256 * 9 * sizeof(uint32_t));
  
  ESP_LOGD(TAG, "Memory allocation for bitmap buffers.");
  ESP_LOGD(TAG, "d_memory_new: %08x p_buffer: %08x.", (unsigned int)d_memory_new, (unsigned int)p_buffer);

  if ((d_memory_new == nullptr) || 
      (p_buffer     == nullptr) ||
      (GLUT         == nullptr) ||
      (GLUT2        == nullptr)) {
    return false;
  }

  Wire::leave();

  d_memory_new->clear();
  memset(p_buffer, 0, BITMAP_SIZE_1BIT * 2);

  calculate_LUTs();

  initialized = true;

  return true;
}

void
EInk10::update(FrameBuffer1Bit & frame_buffer)
{
  ESP_LOGD(TAG, "1bit Update...");
 
  const uint8_t * ptr;
  uint8_t         dram;
  uint8_t         repeat;

  Wire::enter();

  turn_on();

  if (current_waveform_id != INKPLATE10_WAVEFORM1) {
    clean(PixelState::WHITE,     1);
    clean(PixelState::BLACK,    12);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::WHITE,     9);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::BLACK,    12);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::WHITE,     9);
    repeat = 3;
  }
  else {
    clean(PixelState::WHITE,     1);
    clean(PixelState::BLACK,    10);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::WHITE,    10);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::BLACK,    10);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::WHITE,    10);
    repeat = 5;
  }

  uint8_t * data = frame_buffer.get_data();

  for (int k = 0; k < repeat; k++) {

    ptr = &data[BITMAP_SIZE_1BIT - 1];

    vscan_start();

    for (int i = 0; i < HEIGHT; i++) {

      dram = ~(*ptr--);

      hscan_start(PIN_LUT[LUTW[(dram >> 4) & 0x0F]]);
      GPIO.out_w1ts = CL | PIN_LUT[LUTW[dram & 0x0F]];
      GPIO.out_w1tc = CL | DATA;

      for (int j = 0; j < (LINE_SIZE_1BIT - 1); j++) {
        dram = ~(*ptr--);
        GPIO.out_w1ts = CL | PIN_LUT[LUTW[(dram >> 4) & 0x0F]];
        GPIO.out_w1tc = CL | DATA;
        GPIO.out_w1ts = CL | PIN_LUT[LUTW[dram & 0x0F]];
        GPIO.out_w1tc = CL | DATA;
      }

      GPIO.out_w1ts = CL;
      GPIO.out_w1tc = CL| DATA;
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
  allow_partial();
}

void IRAM_ATTR
EInk10::update(FrameBuffer3Bit & frame_buffer)
{
  ESP_LOGD(TAG, "3bit Update...");

  Wire::enter();
  turn_on();

  if (current_waveform_id != INKPLATE10_WAVEFORM1) {
    clean(PixelState::BLACK,     1);
    clean(PixelState::WHITE,     7);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::BLACK,    12);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::WHITE,     7);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::BLACK,    12);
  }
  else {
    clean(PixelState::BLACK,     1);
    clean(PixelState::WHITE,    10);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::BLACK,    10);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::WHITE,    10);
    clean(PixelState::DISCHARGE, 1);
    clean(PixelState::BLACK,    10);
  }

  uint8_t * data = frame_buffer.get_data();

  for (int k = 0, kk = 0; k < 9; k++, kk += 256) {

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
EInk10::partial_update(FrameBuffer1Bit & frame_buffer, bool force)
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
  uint32_t pos = BITMAP_SIZE_1BIT - 1;
  
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < LINE_SIZE_1BIT; j++) {
      uint8_t diffw =  odata[pos] & ~idata[pos];
      uint8_t diffb = ~odata[pos] &  idata[pos];
      pos--;
      p_buffer[n--] = LUTW[diffw >>   4] & (LUTB[diffb >>   4]);
      p_buffer[n--] = LUTW[diffw & 0x0F] & (LUTB[diffb & 0x0F]);
    }
  }

  turn_on();

  uint8_t repeat = (current_waveform_id != INKPLATE10_WAVEFORM1) ? 4 : 5;
 
  for (int k = 0; k < repeat; k++) {
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
EInk10::clean(PixelState pixel_state, uint8_t repeat_count)
{
  turn_on();

  uint32_t send = PIN_LUT[(uint8_t) pixel_state];

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

/**
 * @brief Calculation of LUTs for fast conversion pixels to waveform
 */
void 
EInk10::calculate_LUTs()
{
  for (int i = 0; i < 9; ++i) {
    for (uint32_t j = 0; j < 256; ++j) {
      uint8_t z = (waveform_3bit[j & 0x07][i] << 2) | (waveform_3bit[(j >> 4) & 0x07][i]);
      GLUT[i * 256 + j] = PIN_LUT[z];
      z = ((waveform_3bit[j & 0x07][i] << 2) | (waveform_3bit[(j >> 4) & 0x07][i])) << 4;
      GLUT2[i * 256 + j] = PIN_LUT[z];
    }
  }
}

/**
 * @brief       Function calculates checksum of wavefrom data read from nvs
 *
 * @param       waveformData * w
 *              Structure for waveform data read from nvs. 
 *              Struct can be found in eink_10.hpp class definition.
 *
 * @return      Value of checksum from data read from nvs 'eeprom' segment
 */
uint8_t 
EInk10::calculate_checksum(waveformData * w)
{
  uint8_t * d   = (uint8_t *) w;
  uint16_t  sum = 0;

  for (int i = 0; i < sizeof(waveformData) - 1; i++) {
    sum += d[i];
  }
  return sum % 256;
}

/**
 * @brief       Function reads waveform data from nvs 'eeprom' segment and checks it's validity.
 *
 * @param       waveformData * w
 *              Pointer to structure for waveform data read from nvs 'eeprom'. 
 *              Struct can be found in eink_10.hpp class definition.
 *
 * @return      True if data is vaild, false if not
 */
bool 
EInk10::get_waveform_from_EEPROM(waveformData * w)
{
  if (!nvs_mgr.get((char *) "eeprom", (uint8_t *) w, sizeof(waveformData))) return false;

  return calculate_checksum(w) == w->checksum;
}

bool 
EInk10::burn_waveform_to_EEPROM(waveformData * w, size_t eeprom_size)
{
  w->checksum = calculate_checksum(w);
  return nvs_mgr.put((char *) "eeprom", eeprom_size, (uint8_t *) w, sizeof(waveformData));
}

/**
 * Function allows grayscale waveform to be changed
 *
 * @param       uint8_t * w
 *              Waveform array with 8 rows where every row represents one color and 9 columns where every column
 *              represents one phase or frame of each color.
 */
void 
EInk10::change_waveform(uint8_t * w)
{
  memcpy(waveform_3bit, w, sizeof(waveform_3bit));
  calculate_LUTs();
}

#endif