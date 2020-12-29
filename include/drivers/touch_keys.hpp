#ifndef __TOUCH_KEYS_HPP__
#define __TOUCH_KEYS_HPP__

#include "non_copyable.hpp"
#include "mcp23017.hpp"

class TouchKeys : NonCopyable 
{
  public:
    enum class Key : uint8_t { KEY_0, KEY_1, KEY_2 };

    TouchKeys(MCP23017 & _mcp) : mcp(_mcp) {}
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
    MCP23017 & mcp;

    const MCP23017::Pin TOUCH_0 = MCP23017::Pin::IOPIN_10;
    const MCP23017::Pin TOUCH_1 = MCP23017::Pin::IOPIN_11;
    const MCP23017::Pin TOUCH_2 = MCP23017::Pin::IOPIN_12;

};

#endif