#pragma once

#include "non_copyable.hpp"

#if PCAL6416
  #include "pcal6416.hpp"
#else
  #include "mcp23017.hpp"
#endif

class PressKeys : NonCopyable 
{
  public:
    enum class Key : uint8_t { U2, U4, U1, U5, U3, U6 };

    static const gpio_num_t INTERRUPT_PIN = GPIO_NUM_34;

    PressKeys(IOExpander & _io_expander) : io_expander(_io_expander) {}
    bool setup();

    /**
     * @brief Read a specific presskey
     * 
     * Read one of the six presskeys.
     * 
     * @param key presskey  (U1 .. U6)
     * @return uint8_t 1 if pressed, 0 if not
     */
    uint8_t read_key(Key key);

    /**
     * @brief Read all keys
     * 
     * @return uint8_t keys bitmap (bit = 1 if pressed) keys 0..5 are mapped to bits 0 to 5.
     */
    uint8_t read_all_keys();

  private:
    static constexpr char const * TAG = "PressKeys";
    IOExpander & io_expander;

    const IOExpander::Pin PRESS_0 = IOExpander::Pin::IOPIN_10;
    const IOExpander::Pin PRESS_1 = IOExpander::Pin::IOPIN_11;
    const IOExpander::Pin PRESS_2 = IOExpander::Pin::IOPIN_12;
    const IOExpander::Pin PRESS_3 = IOExpander::Pin::IOPIN_13;
    const IOExpander::Pin PRESS_4 = IOExpander::Pin::IOPIN_14;
    const IOExpander::Pin PRESS_5 = IOExpander::Pin::IOPIN_15;

};
