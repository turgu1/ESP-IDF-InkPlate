#pragma once

#include "non_copyable.hpp"

#if PCAL6416
  #include "pcal6416.hpp"
#else
  #include "mcp23017.hpp"
#endif

class TouchKeys : NonCopyable 
{
  public:
    enum class Key : uint8_t { KEY_0, KEY_1, KEY_2 };

    static const gpio_num_t INTERRUPT_PIN = GPIO_NUM_34;

   TouchKeys(IOExpander & _io_expander) : io_expander(_io_expander) {}
    bool setup();

    /**
     * @brief Read a specific touchkey
     * 
     * Read one of the three touchkeys.
     * 
     * @param key touchkey  (KEY_0, KEY_1, or KEY_2)
     * @return uint8_t 1 if touched, 0 if not
     */
    uint8_t read_key(Key key);

    /**
     * @brief Read all keys
     * 
     * @return uint8_t keys bitmap (bit = 1 if touched) keys 0, 1 and 2 are mapped to bits 0, 1 and 2.
     */
    uint8_t read_all_keys();

  private:
    static constexpr char const * TAG = "TouchKeys";
    IOExpander & io_expander;

    const IOExpander::Pin TOUCH_0 = IOExpander::Pin::IOPIN_10;
    const IOExpander::Pin TOUCH_1 = IOExpander::Pin::IOPIN_11;
    const IOExpander::Pin TOUCH_2 = IOExpander::Pin::IOPIN_12;

};
