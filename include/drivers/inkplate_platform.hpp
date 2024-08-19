/*
inkplate_platform.hpp

Modified by Guy Turcotte
July 20, 2024

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

#pragma once

#include <cstdint>

#include "non_copyable.hpp"

#include "battery.hpp"
#include "eink.hpp"
#include "eink_6.hpp"
#include "eink_6plus.hpp"
#include "eink_6plus_v2.hpp"
#include "eink_10.hpp"
#include "rtc_pcf85063.hpp" 

#if PCAL6416
  #include "pcal6416.hpp"
#else
  #include "mcp23017.hpp"
#endif

#if EXTENDED_CASE && (INKPLATE_6 || INKPLATE_10)
  #include "press_keys.hpp"
#elif INKPLATE_6 || INKPLATE_10
  #include "touch_keys.hpp"
#elif INKPLATE_6PLUS || INKPLATE_6PLUS_V2
  #include "touch_screen.hpp"
  #include "front_light.hpp"
#endif

#if __INKPLATE_PLATFORM__
  IOExpander  io_expander_int(0x20);
  Battery   battery(io_expander_int);
  #if EXTENDED_CASE && (INKPLATE_6 || INKPLATE_10)
    PressKeys press_keys(io_expander_int);
  #elif INKPLATE_6 || INKPLATE_10
    TouchKeys touch_keys(io_expander_int);
  #elif INKPLATE_6PLUS || INKPLATE_6PLUS_V2
    TouchScreen touch_screen(io_expander_int);
    FrontLight   front_light(io_expander_int);
  #endif

  #if INKPLATE_6
    EInk6     e_ink(io_expander_int);
  #elif INKPLATE_10
    IOExpander  io_expander_ext(0x22);
    EInk10    e_ink(io_expander_int, io_expander_ext);
  #elif INKPLATE_6PLUS
    IOExpander  io_expander_ext(0x22);
    EInk6PLUS e_ink(io_expander_int, io_expander_ext);
  #elif INKPLATE_6PLUS_V2
    IOExpander  io_expander_ext(0x22);
    EInk6PLUSV2 e_ink(io_expander_int, io_expander_ext);  
  #else
    #error "One of INKPLATE_6, INKPLATE_10, INKPLATE_6PLUS, INKPLATE_6PLUS_V2 must be defined."
  #endif
  
  RTC       rtc(0x51);
#else
  extern IOExpander  io_expander_int;
  extern Battery   battery;
  #if EXTENDED_CASE
    extern PressKeys press_keys;
  #elif INKPLATE_6 || INKPLATE_10
    extern TouchKeys touch_keys;
  #elif INKPLATE_6PLUS || INKPLATE_6PLUS_V2
    extern TouchScreen touch_screen;
    extern FrontLight   front_light;
  #endif

  #if INKPLATE_6
    extern EInk6     e_ink;
  #elif INKPLATE_10
    extern IOExpander  io_expander_ext;
    extern EInk10    e_ink;
  #elif INKPLATE_6PLUS
    extern IOExpander  io_expander_ext;
    extern EInk6PLUS e_ink;
  #elif INKPLATE_6PLUS_V2
    extern IOExpander  io_expander_ext;
    extern EInk6PLUSV2 e_ink;
  #else
    #error "One of INKPLATE_6, INKPLATE_10, INKPLATE_6PLUS, INKPLATE_6PLUS_V2 must be defined."
  #endif
 
  extern RTC       rtc;
#endif

class InkPlatePlatform : NonCopyable
{
  private:
    static constexpr char const * TAG = "InkPlatePlatform";
    
    static InkPlatePlatform singleton;
    InkPlatePlatform() {};

  public:
    static inline InkPlatePlatform & get_singleton() noexcept { return singleton; }

    /**
     * @brief Setup the InkPlate Devices
     * 
     * This method initialize the SD-Card, the e-Ink display, battery status, and the touchkeys/touchscreen 
     * capabilities.
     * @param sd_card_init - true will initialize the sd_card, default is false
     * @param touch_screen_handler - For INKPLATE_6PLUS only, pointer to the touch_screen handler. As this 
     *                               handler is called from an interrupt function, it is very limited
     *                               in actions that can be taken there. In the handler code, better use
     *                               a flag or a FreeRTOS queue to get the processing done in an out of interrupt
     *                               method or handler. 
     * @return true - All devices ready
     * @return false - Some device not initialized properly
     */
    #if INKPLATE_6PLUS || INKPLATE_6PLUS_V2
      bool setup(bool sd_card_init = false, TouchScreen::ISRHandlerPtr touch_screen_handler = nullptr);
    #else
      bool setup(bool sd_card_init = false);
    #endif

    bool light_sleep(uint32_t minutes_to_sleep, gpio_num_t gpio_num = (gpio_num_t) 0, int level = 1);
    void deep_sleep(gpio_num_t gpio_num = (gpio_num_t) 0, int level = 1);
};

#if __INKPLATE_PLATFORM__
  InkPlatePlatform & inkplate_platform = InkPlatePlatform::get_singleton();
#else
  extern InkPlatePlatform & inkplate_platform;
#endif
