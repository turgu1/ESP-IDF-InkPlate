#pragma once

#include "non_copyable.hpp"
#include "mcp23017.hpp"

class PressKeys : NonCopyable 
{
  public:
    enum class Key : uint8_t { U2, U4, U1, U5, U3, U6 };

    PressKeys(MCP23017 & _mcp) : mcp(_mcp) {}
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
    MCP23017 & mcp;

    const MCP23017::Pin PRESS_0 = MCP23017::Pin::IOPIN_10;
    const MCP23017::Pin PRESS_1 = MCP23017::Pin::IOPIN_11;
    const MCP23017::Pin PRESS_2 = MCP23017::Pin::IOPIN_12;
    const MCP23017::Pin PRESS_3 = MCP23017::Pin::IOPIN_13;
    const MCP23017::Pin PRESS_4 = MCP23017::Pin::IOPIN_14;
    const MCP23017::Pin PRESS_5 = MCP23017::Pin::IOPIN_15;

};
