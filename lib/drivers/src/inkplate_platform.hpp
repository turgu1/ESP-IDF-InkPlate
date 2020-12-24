/*
InkPlatePlatform.h
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

#ifndef __INKPLATE_PLATFORM_HPP__
#define __INKPLATE_PLATFORM_HPP__

#include <cstdint>

#include "non_copyable.hpp"

#include "eink.hpp"
#include "eink_6.hpp"
#include "eink_10.hpp"

class InkPlatePlatform : NonCopyable
{
private:
  static constexpr char const * TAG = "InkPlatePlatform";

  static InkPlatePlatform singleton;
  InkPlatePlatform() {};

  public:
    static inline InkPlatePlatform & get_singleton() noexcept { return singleton; }

    /**
     * @brief Setup the InkPlate-6 Controller
     * 
     * This method initialize the SD-Card access, the e-Ink display and the touchkeys 
     * capabilities.
     * 
     * @return true 
     */
    bool setup();

    bool light_sleep(uint32_t minutes_to_sleep);
    void deep_sleep();
};

#if __INKPLATE_PLATFORM__

  InkPlatePlatform & inkplate_platform = InkPlatePlatform::get_singleton();

  #if defined(INKPLATE_6)
    EInk6 & e_ink = EInk6::get_singleton();
  #elif defined(INKPLATE_10)
    EInk10 & e_ink = EInk10::get_singleton();
  #else
    #error "One of INKPLATE_6, INKPLATE_10 must be defined."
  #endif

#else

  extern InkPlatePlatform & inkplate_platform;

  #if defined(INKPLATE_6)
    extern EInk6 & e_ink;
  #elif defined(INKPLATE_10)
    extern EInk10 & e_ink;
  #else
    #error "One of INKPLATE_6, INKPLATE_10 must be defined."
  #endif
#endif

#endif